#include "speculative_scope_cost.h"
#include "../task.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

using namespace std;

SpeculativeScopeCost::SpeculativeScopeCost(const Task &task, int seed, int max_attempts, string domain_file, string problem_file, bool preserve_link):
    SpeculativeScope(task, seed, max_attempts, domain_file, problem_file, preserve_link)
    {
        for (size_t i = 0; i < type_to_object_index.size(); ++i){
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
            ordered_obj_idxs.push_back(costs);
        }
    }


vector<int> SpeculativeScopeCost::sample_scope(){

    bool scope_found = false;

    unordered_set<int> sampled_objects(required_objects.begin(), required_objects.end());
    vector<int> sampled_objects_vec;

    int attempts = 0;

    while (!scope_found){
        attempts ++;
        if (attempts == max_attempts){
            // cout << "NO SCOPE FOUND" << endl;
            return vector<int>();
        }
        for (size_t type_idx = 0; type_idx < samplable_object_types.size(); ++type_idx){

            // cout << "Sampling for type: " << task.type_names[type_idx] << endl;

            if (!samplable_object_types[type_idx]){
                // cout << "Skipping...." << endl;
                continue;
            }

            int scope_size = min(int(scope_idxs[type_idx].size()), object_num[type_idx]);
            // cout << "Scope size: " << scope_size << endl;

            bool increase_idx = true;
            vector<bool> reset_idx(scope_size, false);
            for (int i = 0; i < scope_size; ++i){
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
            if (all_of(reset_idx.begin(), reset_idx.end(), [](bool e){return e;}) && int(scope_idxs[type_idx].size()) < object_num[type_idx]){
                scope_idxs[type_idx] = vector<int>(scope_size+1, 0);
            }
            
            if (type_idx >= ordered_obj_idxs.size()){
                cout << "ERROR: type idx " << type_idx << " out of bounds for ordered_obj_idxs (size " << ordered_obj_idxs[type_idx].size() << ")" << endl;
            }
            // cout << "sampled objects: "; 
            for (auto idx : scope_idxs[type_idx]){
                if (idx >= int(ordered_obj_idxs[type_idx].size())){
                    cout << "ERROR: idx " << idx << " out of bounds for ordered_obj_idxs[" << type_idx << "] (size " << ordered_obj_idxs[type_idx].size() << ")" << endl;
                }
                sampled_objects.insert(ordered_obj_idxs[type_idx][idx].first);
                // cout << ordered_obj_idxs[type_idx][idx].first << " ";
            }
            // cout << endl;
        }

        if (preserve_links){
            for (auto it = sampled_objects.begin(); it != sampled_objects.end(); ++it){
                auto related_obj = related_objects[*it];
                sampled_objects.insert(related_obj.begin(), related_obj.end());
            }
        }

        sampled_objects_vec = vector<int>(sampled_objects.begin(), sampled_objects.end());
        sort(sampled_objects_vec.begin(), sampled_objects_vec.end());

        scope_found = check_scope_unique(sampled_objects_vec);

    }

    attempted_scopes.insert(sampled_objects_vec);
    // cout << "SCOPE FOUND" << endl;
    // for (auto idx : sampled_objects_vec){
    //     cout << idx << " ";
    // }
    // cout << endl;

    return sampled_objects_vec;
}

SpeculativeScopeCost::~SpeculativeScopeCost() {}



