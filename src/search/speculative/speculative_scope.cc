#include "speculative_scope.h"
#include "../task.h"

#include <vector>
#include <iostream>
#include <unordered_set>

using namespace std;

SpeculativeScope::SpeculativeScope(const Task &task, int seed, int max_attempts)
    : task(task), max_attempts(max_attempts){
    srand(seed);
    
    relevant_predicate_idxs = get_relevant_predicate_idxs(task);
    relevant_actions = get_relevant_actions(task, relevant_predicate_idxs);
    required_objects = get_required_objects(task);
    object_count = get_object_count(relevant_actions, task.compute_object_index());
    type_to_object_index = task.compute_object_index();
    attempted_scopes = unordered_set<vector<int>, TupleHash>();
}

Task SpeculativeScope::speculative_scope(vector<int> &object_idxs){
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

vector<ActionSchema> SpeculativeScope::get_relevant_actions(const Task &task, vector<int> &relevant_pred_idxs){
    vector<ActionSchema> relevant_actions = vector<ActionSchema>();
    auto end_pred_idx_it = relevant_pred_idxs.end();
    auto begin_pred_idx_it = relevant_pred_idxs.begin();

    for (auto &action : task.get_action_schemas()){
        for (auto &effect : action.get_effects()){
            if (find(begin_pred_idx_it, end_pred_idx_it, effect.get_predicate_symbol_idx()) != end_pred_idx_it){
                relevant_actions.push_back(action);
                break;
            }
        }
    }

    return relevant_actions;
}

vector<int> SpeculativeScope::get_relevant_predicate_idxs(const Task &task){
    vector<int> relevant_pred_idxs = vector<int>();
    for (auto &goal : task.get_goal().goal){
        relevant_pred_idxs.push_back(goal.get_predicate_index());
    }

    return relevant_pred_idxs;
}

vector<int> SpeculativeScope::get_required_objects(const Task &task){
    vector<int> required_objects = vector<int>();
    for (auto &goal : task.get_goal().goal){
        for (auto obj_idx : goal.get_arguments()){
            required_objects.push_back(task.objects[obj_idx].get_index());
        }
    }

    auto init_relations = task.initial_state.get_relations();
    auto static_relations = task.static_info.get_relations();

    for (size_t i = 0; i < required_objects.size(); i++){
        // check for required objects in initiation state
        for (auto relation : init_relations){
            for (auto tuple : relation.tuples){
                if (find(tuple.begin(), tuple.end(), required_objects[i]) != tuple.end()){
                    for (auto obj : tuple){
                        if (find(required_objects.begin(), required_objects.end(), required_objects[i]) == required_objects.end()){
                            required_objects.push_back(obj);
                        }
                    }
                }
            }
        }
        //check for required objects in static state
        for (auto relation : static_relations){
            for (auto tuple : relation.tuples){
                if (find(tuple.begin(), tuple.end(), required_objects[i]) != tuple.end()){
                    for (auto obj : tuple){
                        if (find(required_objects.begin(), required_objects.end(), required_objects[i]) == required_objects.end()){
                            required_objects.push_back(obj);
                        }
                    }
                }
            }
        }
    }

    return required_objects;
}

vector<int> SpeculativeScope::get_object_count(const vector<ActionSchema> &relevant_actions,
                                               const std::vector<std::vector<int>> &type_to_object_index){
    vector<int> object_type_count = vector<int>(task.type_names.size(), 0);
    // we are counting the number of times each object appears in a parameter, effect or precondition
    // this might result in a count higher than  the number of available objects which needs to be corrected

    for (auto &action : relevant_actions){
        for (auto &param : action.get_parameters()){
            object_type_count[param.type] ++;
        }
        for (auto &precon : action.get_precondition()){
            int pred_sym = precon.get_predicate_symbol_idx();
            Predicate &pred = task.predicates[pred_sym];
            for (auto type : pred.getTypes()){
                object_type_count[type] ++;
            }
        }
        for (auto &effect : action.get_effects()){
            int pred_sym = effect.get_predicate_symbol_idx();
            Predicate &pred = task.predicates[pred_sym];
            for (auto type : pred.getTypes()){
                object_type_count[type] ++;
            }
        }
    }

    // if we have a higher count than number of objects, set the count to the number of objects available
    for (size_t x = 0; x < object_type_count.size(); x++){
        if (object_type_count[x] > int(type_to_object_index[x].size())){
            object_type_count[x] = type_to_object_index[x].size();
        }
    }

    return object_type_count;
}

int sample_range(int min, int max){
    return min + rand() % (max - min + 1);
}

vector<int> SpeculativeScope::sample_scope(){
    bool scope_found = false;
    vector<int> sampled_objects(required_objects);

    int attempts = 0;

    while (!scope_found){
        attempts ++;
        if (attempts == max_attempts){
            return vector<int>();
        }
        for (size_t type_idx = 0; type_idx < object_count.size(); type_idx++){
            int max = type_to_object_index[type_idx].size();
            int num_objects = sample_range(object_count[type_idx], max);
            for (int x = 0; x < num_objects; x++){
                int obj_idx = sample_range(1, max);
                sampled_objects.push_back(type_to_object_index[type_idx][obj_idx-1]);
            }
        }

        sort(sampled_objects.begin(), sampled_objects.end());
        auto it = unique(sampled_objects.begin(), sampled_objects.end());
        sampled_objects.erase(it, sampled_objects.end());

        scope_found = check_scope_unique(sampled_objects);
    }

    attempted_scopes.insert(sampled_objects);

    return sampled_objects;
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
