#include "speculative_scope_random.h"

vector<int> SpeculativeScopeRandom::sample_scope(){
    bool scope_found = false;

    unordered_set<int> sampled_objects(required_objects.begin(), required_objects.end());
    vector<int> sampled_objects_vec;

    int attempts = 0;

    while (!scope_found){
        attempts ++;
        if (attempts == max_attempts){
            return vector<int>();
        }
        for (size_t type_idx = 0; type_idx < object_count.size(); type_idx++){
            if (!samplable_object_types[type_idx]){
                continue;
            }
            int max = type_to_object_index[type_idx].size();
            int num_objects = sample_range(object_count[type_idx], max);
            for (int x = 0; x < num_objects; x++){
                int obj_idx = sample_range(1, max);
                sampled_objects.insert(type_to_object_index[type_idx][obj_idx-1]);
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

SpeculativeScopeRandom::~SpeculativeScopeRandom() {}