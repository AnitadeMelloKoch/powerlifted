#ifndef SPECULATIVE_SCOPE
#define SPECULATIVE_SCOPE
#include "../task.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <tuple>

class SpeculativeScope{
    protected:
        Task task;
        std::vector<ActionSchema> relevant_actions;
        std::vector<int> relevant_predicate_idxs;
        std::vector<bool> negated_predicates;
        std::vector<bool> affirmed_predicates;
        std::vector<int> object_count;
        std::vector<bool> samplable_object_types;
        std::vector<std::vector<int>> type_to_object_index;
        std::unordered_set<std::vector<int>, TupleHash> attempted_scopes; 
        std::unordered_map<int, std::unordered_set<int>> related_objects;
        std::unordered_map<int, int> object_cost; // cost of including this object into scope
        int max_attempts;

        std::vector<ActionSchema> get_relevant_actions(const Task &task, 
                                                       std::unordered_set<int> &relevant_pred_idxs,
                                                       std::vector<bool> &negated_predicates,
                                                       std::vector<bool> &affirmed_predicates);
        std::vector<Object> get_objects(std::vector<int> &sampled_objects);
        std::vector<int> get_required_objects(const Task &task);
        std::tuple<std::unordered_map<int, std::unordered_set<int>>, 
                   std::unordered_map<int,int>> get_related_objects(const Task &task,
                                                                    std::vector<int> relevant_predicate_idxs);
        std::tuple<std::vector<ActionSchema>, 
                   std::vector<int>,
                   std::vector<bool>,
                   std::vector<bool>> scope_actions(const Task &task);
        std::vector<bool> get_samplable_types(const Task &task, 
                                              std::vector<int> &relevant_predicate_idxs);


        DBState update_state(const DBState &original_state, std::vector<int> &obj_map);
        bool check_scope_unique(const std::vector<int> &object_idxs);

    public:
        Task speculative_scope(std::vector<int> &object_idxs, 
                               bool write_pddl_file = false,
                               std::string file_name = "");
        SpeculativeScope(const Task &task, int seed = 42, int max_attempts = 500);
        void dump_stats(const Task &task);
        virtual std::vector<int> sample_scope();

        virtual ~SpeculativeScope();

};


#endif