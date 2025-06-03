#include "speculative_scope_cost.h"
#include "../task.h"

#include <vector>
#include <unordered_map>
#include <algorithm>

using namespace std;

SpeculativeScopeCost::SpeculativeScopeCost(const Task &task, int seed, int max_attempts):
    SpeculativeScopeCost(task, seed, max_attempts)
    {
        for (size_t i = 0; i < type_to_object_index; ++i){
            object_num.push_back(type_to_object_index[i].size());
            scope_idxs.push_back(vector<int>(1,0));

            vector<pair<int, int>> costs;
            for (auto &obj_idx : type_to_object_index[i]){
                costs.push_back(pair<int, int>(obj_idx, object_cost[obj_idx]));
            }
            sort(costs.begin(), costs.end(), 
                [](const pair<int, int>& a, const pair<int, int>& b){
                    return a.second < b.second;
                });
            ordered_obj_idxs.append(costs);
        }
    }


vector<int> SpeculativeScopeCost::sample_scope(){
    bool scope_found = false;

    unordered_set<int> sampled_objects(required_objects.begin(), required_objects.end());
    vector<int> sampled_objects_vec;

    while (!scope_found){
        for (size_t type_idx = 0; type_idx < object_count.size(); ++type_idx){
            if (!samplable_object_types[type_idx]){
                continue;
            }
            
            int scope_size = scope_idxs[type_idx].size();
            bool increase_idx = true;
            vector<bool> reset_idx(scope_size, false);
            for (size_t i = 0; i < scope_size; ++i){
                if (increase_idx){
                    scope_idxs[type_idx][i]++;
                    increase_idx = false;
                } 
                if (scope_idxs[type_idx][i] >= object_num[type_idx]){
                    scope_idxs[type_idx][i] = 0;
                    increase_idx = true;
                    reset_idx[i] = true;
                }
            }
            if (all_of(reset_idx.begin(), reset_idx.end(), [](bool e){return e;})){
                scope_idxs[type_idx] = vector<int>(scope_size+1, 0);
            }

            for (auto idx : scope_idxs[type_idx]){
                sampled_objects.insert(ordered_obj_idxs[type_idx][idx]);
            }
        }

        for (auto it = sampled_objects.begin(); it != sampled_objects.end(); ++it){
            auto related_obj = related_objects[*it];
            sampled_objects.insert(related_obj.begin(), related_obj.end());
        }

        sampled_objects_vec = vector<int>(sampled_objects.begin(), sampled_objects.end());
        sort(sampled_objects_vec.begin(), sampled_objects_vec.end());

        scope_found = check_scope_unique(sampled_objects_vec);
    }

    attempted_scopes.insert(sampled_objects_vec);
    return sampled_objects_vec;
}

SpeculativeScopeCost::~SpeculativeScopeCost() {}



