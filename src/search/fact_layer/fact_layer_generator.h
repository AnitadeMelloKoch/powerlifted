#ifndef FACT_LAYER_GENERATOR
#include "../states/state.h"
#include "../action.h"
#include "../action_schema.h"
#include "../successor_generators/generic_join_successor.h"
#include "../task.h"
#include "../structures.h"
#include "../database/table.h"

#include <vector>
#include <tuple>

class PtrPrecompiledActionData;

class FactLayerGenerator : public GenericJoinSuccessor{
    private:
        void dump_relation_list(std::vector<PtrRelation> relations);
        void print_relation_stats(std::vector<PtrRelation> relations);
        void print_relation_stats(std::vector<Relation> relations);
        bool find_tuple(
            const std::unordered_set<PtrTable::ptr_tuple_t, PtrTupleHash> &tuples_in_relation,
            const std::vector<int> &tuple
        ) const ;

    public:
        StaticInformation static_info;
        std::vector<PtrPrecompiledActionData> action_data;

        using GenericJoinSuccessor::parse_precond_into_join_program;
        using GenericJoinSuccessor::instantiate;
        using GenericJoinSuccessor::get_applicable_actions;

        explicit FactLayerGenerator(const Task &task);

        std::tuple<std::vector<PtrRelation>, bool> generate_next_fact_layer(
            const std::vector<ActionSchema> action_schemas,
            std::vector<PtrRelation> &relations,
            const GoalCondition &goal
        );

        DBState generate_fact_layers(
            const std::vector<ActionSchema> action_schemas,
            const DBState &state,
            const GoalCondition &goal
        );

        std::tuple<std::vector<PtrRelation>, bool> get_new_relation(
            const ActionSchema action,
            const PtrLiftedOperatorId& op,
            std::vector<PtrRelation> &new_relation
        );

        std::vector<PtrLiftedOperatorId> get_applicable_actions(const ActionSchema &action, const std::vector<PtrRelation> &relations);
        
        bool is_ground_action_applicable(const ActionSchema &action, const std::vector<PtrRelation> &relations) const;

        void select_tuples(const std::vector<PtrRelation> &relations,
                           const Atom &a,
                           std::vector<std::shared_ptr<GroundAtom>> &tuples,
                           const std::vector<int> &constants);
        
        bool parse_precond_into_join_program(const PtrPrecompiledActionData &adata,
                                             const std::vector<PtrRelation> &relations,
                                             std::vector<PtrTable>& tables);
        
        PtrTable instantiate(const ActionSchema &action, const std::vector<PtrRelation> &relations);

        bool apply_ground_action_effects(const ActionSchema &action,
                                         std::vector<PtrRelation> &relation);
        
        bool apply_lifted_action_effects(const ActionSchema &action,
                                         const std::shared_ptr<std::vector<int>> tuple,
                                         std::vector<PtrRelation> &relation);
        
        static void filter_static(const ActionSchema &action, PtrTable &working_table);

        const GroundAtom tuple_to_atom(const std::shared_ptr<std::vector<int>> tuple, const Atom &eff);

        bool check_goal(const std::vector<PtrRelation> &relations, const GoalCondition &goal);

    protected:
        static void order_tuple_by_free_variable_order(const std::vector<int> &free_var_indices,
                                                       const std::vector<int> &map_indices_to_position,
                                                       const std::shared_ptr<std::vector<int>> tuple_with_const,
                                                       std::shared_ptr<std::vector<int>> ordered_tuple);

        static void compute_map_indices_to_table_positions(const PtrTable &instantiations,
                                                           std::vector<int> &free_car_indices,
                                                           std::vector<int> &map_indices_to_position);
        
        std::vector<PtrPrecompiledActionData> precompile_action_data(const std::vector<ActionSchema>& action);

        PtrPrecompiledActionData precompile_action_data(const ActionSchema& action);
        

};

class PtrPrecompiledActionData{
    public:
        PtrPrecompiledActionData() :
            is_ground(false), statically_inapplicable(false),
            relevant_precondition_atoms(), fluent_tables(),
            precompiled_db()
        {}
    
        bool is_ground;

        bool statically_inapplicable;

        std::vector<Atom> relevant_precondition_atoms;

        std::vector<unsigned> fluent_tables;

        std::vector<PtrTable> precompiled_db;

};

#endif