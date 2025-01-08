#include "speculative_scope.h"
#include "../task.h"

#include <vector>
#include <iostream>

using namespace std;

SpeculativeScope::SpeculativeScope(const Task &task, int seed = 42)
    : task(task){
    srand(seed);
    
    relevant_predicate_idxs = get_relevant_predicate_idxs(task);
    relevant_actions = get_relevant_actions(task, relevant_predicate_idxs);
    required_objects = get_required_objects(task);
    object_count = get_object_count(relevant_actions, task.compute_object_index());
    type_to_object_index = task.compute_object_index();
}

Task SpeculativeScope::speculative_scope(){
    Task new_task = task;

    new_task.objects = sample_objects();
    return new_task;
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

vector<Object> SpeculativeScope::get_required_objects(const Task &task){
    vector<Object> required_objects = vector<Object>();
    for (auto &goal : task.get_goal().goal){
        for (auto obj_idx : goal.get_arguments()){
            required_objects.push_back(task.objects[obj_idx]);
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
        if (object_type_count[x] < type_to_object_index[x].size()){
            object_type_count[x] = type_to_object_index[x].size();
        }
    }

    return object_type_count;
}

int sample_range(int min, int max){
    return min + rand() % (max - min + 1);
}

vector<Object> SpeculativeScope::sample_objects(){
    // The object count is an estimation of the minimum number of objects that could be required
    // this may still be too few objects so randomly select how many objects we grab
    vector<vector<int>> sampled_objects(object_count.size());
    for (size_t type_idx = 0; type_idx < object_count.size(); type_idx++){
        int max = type_to_object_index[type_idx].size();
        int num_objects = sample_range(object_count[type_idx], max);
        for (int x = 0; x < num_objects; x++){
            int obj_idx = sample_range(0, max);
            sampled_objects[type_idx].push_back(type_to_object_index[type_idx][obj_idx]);
        }
    }

    vector<Object> objects = vector<Object>();

    for (auto &obj_list : sampled_objects){
        for (auto &idx : obj_list){
            objects.push_back(task.objects[idx]);
        }
    }
    return objects;
}
