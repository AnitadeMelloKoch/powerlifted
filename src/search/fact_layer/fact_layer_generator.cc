#include "fact_layer_generator.h"
#include "../task.h"
#include "../successor_generators/generic_join_successor.h"
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

FactLayerGenerator::FactLayerGenerator(const Task &task) 
    : GenericJoinSuccessor(task), static_info(task.static_info){
}

void FactLayerGenerator::dump_relation_list(std::vector<Relation> relations){
    std::cout << "relations size: " << relations.size() << " " << std::endl;
    for(size_t i = 0; i < relations.size(); i++){
        std::unordered_set<GroundAtom, TupleHash> tuples = relations[i].tuples;
        std::cout << "relation predicate: " << relations[i].predicate_symbol << " ";
        for (auto &tuple:tuples) {
            std::cout << "(";
            for (auto obj : tuple){
                std::cout << obj << ",";
            }
            std::cout << "), ";
        }
        std::cout << " | ";
    }
    std::cout << std::endl;
}

DBState FactLayerGenerator::generate_next_fact_layer(
    const std::vector<LiftedOperatorId> &ops,
    const std::vector<ActionSchema> action_schemas,
    const DBState &state
){
    std::vector<bool> new_nullary_atoms(state.get_nullary_atoms());
    std::vector<Relation> relations(state.get_relations());

    for (auto & op : ops){
        const auto &action = action_schemas[op.get_index()];
        std::vector<Relation> new_relations(state.get_relations());
        new_relations = get_new_relation(action, op, new_relations);

        for (auto relation : new_relations){
            for (size_t i = 0; i < relations.size(); i++){
                if (relations[i].predicate_symbol == relation.predicate_symbol){
                    for (auto tuple:relation.tuples){
                        if (relations[i].tuples.find(tuple) == relations[i].tuples.end()){
                            relations[i].tuples.insert(tuple);
                        }
                    }
                }
            }
        }
        apply_nullary_effects(action, new_nullary_atoms);
    }

    return DBState(std::move(relations), std::move(new_nullary_atoms));
    
}

std::vector<Relation> FactLayerGenerator::get_new_relation(
    const ActionSchema action,
    const LiftedOperatorId& op,
    std::vector<Relation> new_relation
){
    if (action.is_ground()){
        apply_ground_action_effects(action, new_relation);
    }
    else{
        apply_lifted_action_effects(action, op.get_instantiation(), new_relation);
    }
    return new_relation;
}



