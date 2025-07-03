#include "speculative_scope_cost.h"
#include "../task.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

using namespace std;

SpeculativeScopeCost::SpeculativeScopeCost(const Task &task, int seed, int max_attempts, string domain_file):
    SpeculativeScope(task, seed, max_attempts, domain_file)
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
    // cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;

    bool scope_found = false;

    unordered_set<int> sampled_objects(required_objects.begin(), required_objects.end());
    vector<int> sampled_objects_vec;

    int attempts = 0;

    while (!scope_found){
        attempts ++;
        if (attempts == max_attempts){
            return vector<int>();
        }
        for (size_t type_idx = 0; type_idx < samplable_object_types.size(); ++type_idx){

            if (type_idx >= scope_idxs.size()) {
                cout << "🔥 type_idx " << type_idx << " out of bounds for scope_idxs (size "
                    << scope_idxs.size() << ")" << endl;
            }



            // cout << "============ TYPE IDX " << type_idx << " ============" << endl;

            // cout << "sample object type size: " << samplable_object_types.size() << endl; 

            if (!samplable_object_types[type_idx]){
                // cout << "not samplable type. Skip sampling" << endl;
                continue;
            }
            
            // cout << "scope idx size: " << scope_idxs[type_idx].size() << " num type of objects: " << object_num[type_idx] << endl;
            // cout << "sample indexs: ";
            // for (auto idx : scope_idxs[type_idx]){
            //     cout << idx << " ";
            // }
            // cout << endl;

            int scope_size = min(int(scope_idxs[type_idx].size()), object_num[type_idx]);

            // cout << "scope size: " << scope_size << endl;

            bool increase_idx = true;
            vector<bool> reset_idx(scope_size, false);
            for (int i = 0; i < scope_size; ++i){
                if (increase_idx){
                    // cout << "increase index" << endl;
                    scope_idxs[type_idx][i]++;
                    increase_idx = false;
                } 
                if (scope_idxs[type_idx][i] >= object_num[type_idx]){
                    // cout << "reset scope size" << endl;
                    scope_idxs[type_idx][i] = 0;
                    increase_idx = true;
                    reset_idx[i] = true;
                }
            }
            if (all_of(reset_idx.begin(), reset_idx.end(), [](bool e){return e;}) && int(scope_idxs[type_idx].size()) < object_num[type_idx]){
                // cout << "all scope idx reset so increase scope size" << endl;
                scope_idxs[type_idx] = vector<int>(scope_size+1, 0);
            }
            
            // cout << "get all sampled objects by index" << endl;
            if (type_idx >= ordered_obj_idxs.size()){
                cout << "ERROR: type idx " << type_idx << " out of bounds for ordered_obj_idxs (size " << ordered_obj_idxs[type_idx].size() << ")" << endl;
            }
            for (auto idx : scope_idxs[type_idx]){
                if (idx >= int(ordered_obj_idxs[type_idx].size())){
                    cout << "ERROR: idx " << idx << " out of bounds for ordered_obj_idxs[" << type_idx << "] (size " << ordered_obj_idxs[type_idx].size() << ")" << endl;
                }
                sampled_objects.insert(ordered_obj_idxs[type_idx][idx].first);
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



