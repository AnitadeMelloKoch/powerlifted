#ifndef FACT_LAYER_GENERATOR
#include "../states/state.h"
#include "../action.h"
#include "../action_schema.h"
#include "../successor_generators/generic_join_successor.h"
#include "../task.h"
#include "../structures.h"
#include "../database/table.h"

#include <vector>


class FactLayerGenerator : public GenericJoinSuccessor{
    private:
        void dump_relation_list(std::vector<Relation> relations);

    public:
        StaticInformation static_info;

        explicit FactLayerGenerator(const Task &task);

        DBState generate_next_fact_layer(
            const std::vector<LiftedOperatorId> &ops,
            const std::vector<ActionSchema> action_schemas,
            const DBState &state
        );

        std::vector<Relation> get_new_relation(
            const ActionSchema action,
            const LiftedOperatorId& op,
            std::vector<Relation> new_relation
        );

};



#endif