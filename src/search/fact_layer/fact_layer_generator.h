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


class FactLayerGenerator : public GenericJoinSuccessor{
    private:
        void dump_relation_list(std::vector<Relation> relations);

    public:
        StaticInformation static_info;

        using GenericJoinSuccessor::parse_precond_into_join_program;
        using GenericJoinSuccessor::instantiate;
        using GenericJoinSuccessor::get_applicable_actions;

        explicit FactLayerGenerator(const Task &task);

        std::tuple<std::vector<Relation>, bool> generate_next_fact_layer(
            const std::vector<ActionSchema> action_schemas,
            std::vector<Relation> relations
        );

        DBState generate_fact_layers(
            const std::vector<ActionSchema> action_schemas,
            const DBState &state
        );

        std::vector<Relation> get_new_relation(
            const ActionSchema action,
            const LiftedOperatorId& op,
            std::vector<Relation> &new_relation
        );

        std::tuple<std::vector<Relation>, bool> add_new_relations(
            std::vector<Relation> &relations,
            std::vector<Relation> &new_relations
        );

        std::vector<LiftedOperatorId> get_applicable_actions(const ActionSchema &action, const std::vector<Relation> relations);
        
        bool is_ground_action_applicable(const ActionSchema &action, const std::vector<Relation> relations) const;

        void select_tuples(const std::vector<Relation> relations,
                           const Atom &a,
                           std::vector<GroundAtom> &tuples,
                           const std::vector<int> &constants);
        
        bool parse_precond_into_join_program(const PrecompiledActionData &adata,
                                             const std::vector<Relation> relations,
                                             std::vector<Table>& tables);
        
        Table instantiate(const ActionSchema &action, const std::vector<Relation> relations);

};



#endif