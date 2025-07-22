#include "speculative_scope.h"
#include "../task.h"
#include "../writer.h"

#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <tuple>

using namespace std;

SpeculativeScope::SpeculativeScope(const Task &task, int seed, int max_attempts, string domain_file, string problem_file)
    : task(task), max_attempts(max_attempts), writer(domain_file, problem_file){
    srand(seed);
    
    cout << "initialising" << endl;
    cout << "action num: " << task.get_action_schemas().size() << endl; 
    for (auto action : task.get_action_schemas()){
        cout << action.get_name() << endl;
    }

    tie(relevant_actions,
        relevant_predicate_idxs,
        negated_predicates,
        affirmed_predicates) = scope_actions(task);
    samplable_object_types = get_samplable_types(task,
                                                 relevant_predicate_idxs,
                                                 relevant_actions);
    required_objects = get_required_objects(task);
    type_to_object_index = task.compute_object_index();
    tie(related_objects, object_cost) = get_related_objects(task,
                                                            relevant_predicate_idxs);
    attempted_scopes = unordered_set<vector<int>, TupleHash>();
}

SpeculativeScope::~SpeculativeScope() {}

vector<int> SpeculativeScope::sample_scope(){
    throw runtime_error("sample scope not implemented");
}

Task SpeculativeScope::speculative_scope(vector<int> &object_idxs, 
                                         bool write_pddl_file,
                                        string file_name){
    Task new_task = task;

    auto sampled_objects = get_objects(object_idxs);

    new_task.objects = vector<Object>();

    vector<int> old_obj_to_new_map(sampled_objects.size());

    for (size_t x = 0; x < sampled_objects.size(); x++){
        new_task.add_object(sampled_objects[x].get_name(),
                            x,
                            sampled_objects[x].get_types());
        old_obj_to_new_map[x] = sampled_objects[x].get_index();
    }

    new_task.initial_state = update_state(new_task.initial_state, old_obj_to_new_map);
    new_task.static_info = update_state(new_task.static_info, old_obj_to_new_map);

    auto map_begin = old_obj_to_new_map.begin();
    auto map_end = old_obj_to_new_map.end();

    GoalCondition old_goal = new_task.get_goal();
    vector<AtomicGoal> new_goals = vector<AtomicGoal>();
    for (auto &goal : old_goal.goal){
        auto args = goal.get_arguments();
        vector<int> new_args = vector<int>();
        for (auto arg : args){
            auto obj_idx = find(map_begin, map_end, arg);
            new_args.push_back(distance(map_begin, obj_idx));
        }
        new_goals.push_back(AtomicGoal(goal.get_predicate_index(), new_args, goal.is_negated()));
    }

    new_task.create_goal_condition(
        new_goals,
        old_goal.positive_nullary_goals,
        old_goal.negative_nullary_goals
    );

    if (write_pddl_file){
        if (file_name == ""){
            throw invalid_argument("Filename was not provided but write_to_file is true.");
        }
        auto success = write(new_task, file_name);
        if (!success){
            throw runtime_error("Error during file write for file: " + file_name);
        }
    }
    
    return new_task;
}

DBState SpeculativeScope::update_state(const DBState &original_state, vector<int> &obj_map){
    vector<Relation> new_relations = vector<Relation>();
    vector<Relation> old_relations = original_state.get_relations();
    auto map_begin = obj_map.begin();
    auto map_end = obj_map.end();
    for (auto relation : old_relations){
        unordered_set<GroundAtom, TupleHash> new_tuples = unordered_set<GroundAtom, TupleHash>();
        for (auto tuple : relation.tuples){
            vector<int> new_tuple = vector<int>();
            for (auto idx : tuple){
                auto obj_it = find(map_begin, map_end, idx);
                if (obj_it != map_end){
                    new_tuple.push_back(distance(map_begin, obj_it));
                } else {
                    break;
                }
            }
            if (new_tuple.size() == tuple.size()){
                new_tuples.insert(new_tuple);
            }
        }
        new_relations.push_back(Relation(relation.predicate_symbol, move(new_tuples)));
    }

    vector<bool> new_nullary_atoms(original_state.get_nullary_atoms());

    return DBState(move(new_relations), move(new_nullary_atoms));
}

// TODO REALLY NEED TO CHECK THESE FUNCTIONS WORK WITH NULLARY ATOMS

vector<ActionSchema> SpeculativeScope::get_relevant_actions(const Task &task, 
                                                            unordered_set<int> &relevant_pred_idxs,
                                                            vector<bool> &negated_predicates,
                                                            vector<bool> &affirmed_predicates){
    vector<ActionSchema> relevant_actions = vector<ActionSchema>();
    auto end_pred_idx_it = relevant_pred_idxs.end();
    auto begin_pred_idx_it = relevant_pred_idxs.begin();

    for (auto &action : task.get_action_schemas()){
        bool action_added = false;
        for (size_t i = 0; i < action.get_positive_nullary_effects().size(); ++i){
            if (action.get_positive_nullary_effects()[i]){
                if (affirmed_predicates[i]){
                    relevant_actions.push_back(action);
                    action_added = true;
                    break;
                }
            }
        }

        if (action_added){
            continue;
        }

        for (size_t i = 0; i < action.get_negative_nullary_effects().size(); ++i){
            if (action.get_negative_nullary_effects()[i]){
                if (negated_predicates[i]){
                    relevant_actions.push_back(action);
                    action_added = true;
                    break;
                }
            }
        }

        if (action_added){
            continue;
        }

        for (auto &effect : action.get_effects()){
            if (find(begin_pred_idx_it, end_pred_idx_it, effect.get_predicate_symbol_idx()) != end_pred_idx_it){
                if (effect.is_negated()){
                    if (negated_predicates[effect.get_predicate_symbol_idx()]){
                        relevant_actions.push_back(action);
                        break;
                    }
                } else {
                    if (affirmed_predicates[effect.get_predicate_symbol_idx()]){
                        relevant_actions.push_back(action);
                        break;
                    }
                }
            }
        }
    }

    return relevant_actions;
}

tuple<vector<ActionSchema>, vector<int>, vector<bool>, vector<bool>> SpeculativeScope::scope_actions(const Task &task){
    unordered_set<int> relevant_predicates;
    vector<ActionSchema> relevant_actions;
    // we should track whether predicate is negated or not so we know which 
    // version of predicates should be preserved
    vector<bool> negated_predicate(task.predicates.size(), false);
    vector<bool> affirmed_predicate(task.predicates.size(), false);

    bool action_added = false;

    // get effects from each atomic goal
    for (auto &goal : task.get_goal().goal){
        relevant_predicates.insert(goal.get_predicate_index());
        if (goal.is_negated()){
            negated_predicate[goal.get_predicate_index()] = true;
        } else {
            affirmed_predicate[goal.get_predicate_index()] = true;
        }
    }

    // get positive and negative nullary goals
    for (auto &pred_idx : task.get_goal().positive_nullary_goals){
        affirmed_predicate[pred_idx] = true;
        relevant_predicates.insert(pred_idx);
    }
    for (auto &pred_idx : task.get_goal().negative_nullary_goals){
        negated_predicate[pred_idx] = true;
        relevant_predicates.insert(pred_idx);
    }

    // cout << "initial predicates" << endl;
    // for (auto idx : relevant_predicates){
    //     cout << idx << " ";
    // }
    // cout << endl;

    do{
        auto action_num = relevant_actions.size();
        action_added = false;

        for (ActionSchema &action : relevant_actions){
            for (size_t i = 0; i < action.get_positive_nullary_precond().size(); ++i){
                if(action.get_negative_nullary_precond()[i]){
                    negated_predicate[i] = true;
                    relevant_predicates.insert(i);
                }
            }

            for (size_t i = 0; i < action.get_positive_nullary_precond().size(); ++i){
                if (action.get_positive_nullary_precond()[i]){
                    affirmed_predicate[i] = true;
                    relevant_predicates.insert(i);
                }
            }

            for (auto &effect : action.get_precondition()){
                relevant_predicates.insert(effect.get_predicate_symbol_idx());
                if (effect.is_negated()){
                    negated_predicate[effect.get_predicate_symbol_idx()] = true;
                } else {
                    affirmed_predicate[effect.get_predicate_symbol_idx()] = true;
                }
            }
        }

        relevant_actions = get_relevant_actions(task, 
                                                relevant_predicates,
                                                negated_predicate,
                                                affirmed_predicate);

        if (action_num < relevant_actions.size()){
            action_added = true;
        }

        // cout << "in loop" << endl;
        // for (auto action : relevant_actions){
        //     cout << action.get_name() << " ";
        // }
        // cout << endl;
        // for (auto idx : relevant_predicates){
        //     cout << idx << " ";
        // }
        // cout << endl;

        
    } while(action_added);

    vector<int> relevant_pred_vec(relevant_predicates.begin(),
                                  relevant_predicates.end());
    cout << "returning" << endl;
    return make_tuple(relevant_actions,
                      relevant_pred_vec,
                      negated_predicate,
                      affirmed_predicate); 
}



vector<int> SpeculativeScope::get_required_objects(const Task &task){
    unordered_set<int> required_objects = unordered_set<int>();
    for (auto &goal : task.get_goal().goal){
        for (auto obj_idx : goal.get_arguments()){
            required_objects.insert(task.objects[obj_idx].get_index());
        }
    }

    auto constants = writer.get_domain_defined_objects();

    for (auto &obj : task.objects){
        if (find(constants.begin(), constants.end(), obj.get_name()) != constants.end()){
            required_objects.insert(obj.get_index());
        }
    } 

    auto required_objects_vec = vector<int>(required_objects.begin(), required_objects.end());
    

    return required_objects_vec;
}

vector<bool> SpeculativeScope::get_samplable_types(const Task &task,
                                                   vector<int> &relevant_predicate_idxs,
                                                   vector<ActionSchema> &relevant_actions){
    // go through all predicates and save what types are important then save ids 
    // of all objects of those types
    vector<bool> object_type_needed(task.type_names.size(), false);
    for (auto relation : task.initial_state.get_relations()){
        if (find(relevant_predicate_idxs.begin(),
                 relevant_predicate_idxs.end(),
                 relation.predicate_symbol) != relevant_predicate_idxs.end()){
            auto predicate = task.predicates[relation.predicate_symbol];
            for (auto type : predicate.getTypes()){
                object_type_needed[type] = true;
            }
        }
    }

    for (auto action : relevant_actions){
        for (auto param : action.get_parameters()){
            object_type_needed[param.type] = true;
        }
    }

    if (object_type_needed.size() > 1){
        for (size_t i = 0; i < task.type_names.size(); ++i){
            if (task.type_names[i] == "object"){
                object_type_needed[i] = false;
            }
        }
    }

    return object_type_needed;
}

bool compare_index(const Object &o1, const Object &o2){
    return o1.get_index() < o2.get_index();
}

tuple<unordered_map<int, unordered_set<int>>,
      unordered_map<int, int>> SpeculativeScope::get_related_objects(const Task &task,
                                                                     vector<int> relevant_predicate_idxs){
    
    auto &objects = task.objects;
    const auto &init_relations = task.initial_state.get_relations();
    const auto &static_relations = task.static_info.get_relations();

    unordered_map<int, unordered_set<int>> related_objects;
    unordered_map<int, int> object_costs;

    for (auto object : objects){
        unordered_set<int> related_obj;
        related_obj.insert(object.get_index());
        bool objects_added = false;

        do {
            auto original_size = related_obj.size();
            objects_added = false;
            for (auto it = related_obj.begin(); it != related_obj.end(); ++it){
                for (auto &relation : init_relations){
                    if (find(relevant_predicate_idxs.begin(),
                             relevant_predicate_idxs.end(),
                             relation.predicate_symbol) != relevant_predicate_idxs.end()){
                        for (auto &tuple : relation.tuples){
                            if (find(tuple.begin(), tuple.end(), *it) != tuple.end()){
                                for (auto obj_id : tuple){
                                    related_obj.insert(obj_id);
                                }
                            }
                        }
                    }
                }
                for (auto &relation : static_relations){
                    if (find(relevant_predicate_idxs.begin(),
                             relevant_predicate_idxs.end(),
                             relation.predicate_symbol) != relevant_predicate_idxs.end()){
                        for (auto &tuple : relation.tuples){
                            if (find(tuple.begin(), tuple.end(), *it) != tuple.end()){
                                for (auto obj_id : tuple){
                                    related_obj.insert(obj_id);
                                }
                            }
                        }
                    }
                }
            }

            if (original_size < related_obj.size()){
                objects_added = true;
            }


        } while(objects_added);

        related_objects[object.get_index()] = related_obj;
        if (find(required_objects.begin(), required_objects.end(), object.get_index()) != required_objects.end()){
            object_costs[object.get_index()] = related_obj.size();
        } else {
            object_cost[object.get_index()] = 0;
        }
    }

    return make_tuple(related_objects, object_costs);
}

int SpeculativeScope::sample_range(int min, int max){
    return min + rand() % (max - min + 1);
}

vector<Object> SpeculativeScope::get_objects(vector<int> &sampled_objects){
    // The object count is an estimation of the minimum number of objects that could be required
    // this may still be too few objects so randomly select how many objects we grab
    vector<Object> objects = vector<Object>();
    for (auto &idx : sampled_objects){
        objects.push_back(task.objects[idx]);
    }

    return objects;
}

bool SpeculativeScope::check_scope_unique(const vector<int> &object_idxs){
    // for (auto idx : object_idxs){
    //     cout << idx << " ";
    // }
    // cout << endl;
    if (attempted_scopes.find(object_idxs)==attempted_scopes.end()){
        return true;
    }
    return false;
}

void SpeculativeScope::dump_stats(const Task &given_task){
    cout << "=====================================================" << endl;
    cout << "_______________ Original Predicates _________________" << endl;
    for (size_t i = 0; i < task.predicates.size(); i++){
        cout << i << " " << task.predicates[i].get_name() << endl;
    }
    cout << "_________________ Original Objects __________________" << endl;
    for (size_t i = 0; i < task.objects.size(); i++){
        cout << i << " " << task.objects[i].get_name() << endl;
    }
    cout << "_________________ Original Actions __________________" << endl;
    for (size_t i = 0; i < task.get_action_schemas().size(); i++){
        cout << i << " " << task.get_action_schemas()[i].get_name() << endl;
    }


    cout << "____________________________________________" << endl;
    cout << "_______________ Predicates _________________" << endl;
    for (size_t i = 0; i < given_task.predicates.size(); i++){
        cout << i << " " << given_task.predicates[i].get_name() << endl;
    }
    cout << "_________________ Objects __________________" << endl;
    for (size_t i = 0; i < given_task.objects.size(); i++){
        cout << i << " " << given_task.objects[i].get_name() << endl;
    }
    cout << "_________________ Actions __________________" << endl;
    for (size_t i = 0; i < given_task.get_action_schemas().size(); i++){
        cout << i << " " << given_task.get_action_schemas()[i].get_name() << endl;
    }
    cout << "=====================================================" << endl;
}

bool SpeculativeScope::write(Task &task, string filename){
    return writer.write(task, filename);
}

bool SpeculativeScope::write_summary(string filename,
                                     int scope_num,
                                     bool task_success,
                                     std::chrono::time_point<std::chrono::high_resolution_clock> start,
                                     std::chrono::time_point<std::chrono::high_resolution_clock> end){
    return writer.write_summary(filename, scope_num, task_success, start, end);
}