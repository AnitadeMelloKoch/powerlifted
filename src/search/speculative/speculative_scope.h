#ifndef SPECULATIVE_SCOPE
#define SPECULATIVE_SCOPE
#include "../task.h"

#include <vector>
#include <unordered_set>

class SpeculativeScope{
    private:
        Task task;
        std::vector<ActionSchema> relevant_actions;
        std::vector<int> relevant_predicate_idxs;
        std::vector<int> required_objects;
        std::vector<int> object_count;
        std::vector<std::vector<int>> type_to_object_index;
        std::unordered_set<std::vector<int>, TupleHash> attempted_scopes; 
        int max_attempts;

        std::vector<int> get_relevant_predicate_idxs(const Task &task);
        std::vector<ActionSchema> get_relevant_actions(const Task &task, std::vector<int> &relevant_pred_idxs);
        std::vector<Object> get_objects(std::vector<int> &sampled_objects);
        std::vector<int> get_object_count(const std::vector<ActionSchema> &relevant_actions, 
                                          const std::vector<std::vector<int>> &type_to_object_index);
        std::vector<int> get_required_objects(const Task &task);
        DBState update_state(const DBState &original_state, std::vector<int> &obj_map);
        bool check_scope_unique(const std::vector<int> &object_idxs);
    
    public:
        Task speculative_scope(std::vector<int> &object_idxs);
        SpeculativeScope(const Task &task, int seed = 42, int max_attempts = 500);
        void dump_stats(const Task &task);
        std::vector<int> sample_scope();

};


#endif