#ifndef SPECULATIVE_SCOPE
#include "../task.h"

#include <vector>

class SpeculativeScope{
    private:
        Task task;
        std::vector<ActionSchema> relevant_actions;
        std::vector<int> relevant_predicate_idxs;
        std::vector<Object> required_objects;
        std::vector<int> object_count;
        std::vector<std::vector<int>> type_to_object_index;

        std::vector<int> get_relevant_predicate_idxs(const Task &task);
        std::vector<ActionSchema> get_relevant_actions(const Task &task, std::vector<int> &relevant_pred_idxs);
        // will need a way to track what combinations of objects have been tried
        std::vector<Object> sample_objects();
        std::vector<int> get_object_count(const std::vector<ActionSchema> &relevant_actions, 
                                          const std::vector<std::vector<int>> &type_to_object_index);
        std::vector<Object> get_required_objects(const Task &task);

    
    public:
        Task speculative_scope();
        explicit SpeculativeScope(const Task &task, int seed = 42);

};


#endif