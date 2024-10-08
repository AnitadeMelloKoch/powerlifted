#include "fact_layer_generator.h"
#include "../task.h"
#include "../successor_generators/generic_join_successor.h"
#include "../database/hash_join.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <cassert>

using namespace std;

FactLayerGenerator::FactLayerGenerator(const Task &task) 
    : GenericJoinSuccessor(task), static_info(task.static_info){
}

void FactLayerGenerator::dump_relation_list(std::vector<Relation> relations){
    cout << "relations size: " << relations.size() << " " << endl;
    for(size_t i = 0; i < relations.size(); i++){
        unordered_set<GroundAtom, TupleHash> tuples = relations[i].tuples;
        cout << "relation predicate: " << relations[i].predicate_symbol << " ";
        for (auto &tuple:tuples) {
            cout << "(";
            for (auto obj : tuple){
                cout << obj << ",";
            }
            cout << "), ";
        }
        cout << " | ";
    }
    cout << endl;
}

// std::tuple<DBState, bool> FactLayerGenerator::generate_next_fact_layer(
//     const std::vector<ActionSchema> action_schemas,
//     const DBState &state
// ){
//     std::vector<bool> new_nullary_atoms(state.get_nullary_atoms());
//     std::vector<Relation> relations(state.get_relations());

//     bool extended = false;

//     for (auto & action : action_schemas){
//         std::vector<Relation> new_relations(state.get_relations());
        
//         new_relations = get_new_relation(action, op, new_relations);

//         for (auto relation : new_relations){
//             for (size_t i = 0; i < relations.size(); i++){
//                 if (relations[i].predicate_symbol == relation.predicate_symbol){
//                     for (auto tuple:relation.tuples){
//                         if (relations[i].tuples.find(tuple) == relations[i].tuples.end()){
//                             relations[i].tuples.insert(tuple);
//                             extended = true;
//                         }
//                     }
//                 }
//             }
//         }
//         apply_nullary_effects(action, new_nullary_atoms);
//     }

//     return std::make_tuple(DBState(std::move(relations), std::move(new_nullary_atoms)), extended);
    
// }

// Need to figure out where all grounded actions are then work out
// how to reapply those actions

tuple<vector<Relation>, bool> FactLayerGenerator::generate_next_fact_layer(
    const std::vector<ActionSchema> action_schemas,
    vector<Relation> relations
){
    bool extended = false;

    for (auto &action : action_schemas){
        auto applicable = get_applicable_actions(action, relations);
        for (auto &op : applicable){
            vector<Relation> new_relation(relations);
            new_relation = get_new_relation(action,
                                            op, 
                                            new_relation);
            bool op_extended;
            tie(relations, op_extended) = add_new_relations(relations, new_relation);
            extended = extended | op_extended;
        }
    }

    return make_tuple(relations, extended);
}

void FactLayerGenerator::generate_fact_layers(const vector<ActionSchema> action_schemas, 
                                              const DBState &state){
    vector<bool> new_nullary_atoms(state.get_nullary_atoms());
    vector<Relation> relations(state.get_relations());

    bool extended = true;
    while (extended){
        tie(relations, extended) = generate_next_fact_layer(action_schemas, relations);
        cout << "extended: " << extended << endl; 
    }
    cout << "+++++++++++++++++++++++++++++" << endl;
    dump_relation_list(relations);
    cout << "+++++++++++++++++++++++++++++" << endl;
}

vector<Relation> FactLayerGenerator::get_new_relation(
    const ActionSchema action,
    const LiftedOperatorId& op,
    vector<Relation> &new_relation
){
    if (action.is_ground()){
        apply_ground_action_effects(action, new_relation);
    }
    else{
        apply_lifted_action_effects(action, op.get_instantiation(), new_relation);
    }
    return new_relation;
}

tuple<vector<Relation>, bool> FactLayerGenerator::add_new_relations(
    vector<Relation> &relations,
    vector<Relation> &new_relations
){
    bool extended = false;
    for (auto new_relation : new_relations){
        for (size_t i = 0; i < relations.size(); i++){
            if (relations[i].predicate_symbol == new_relation.predicate_symbol){
                for (auto tuple : new_relation.tuples){
                    // tuple does not exist yet so need to add it
                    if (relations[i].tuples.find(tuple) == relations[i].tuples.end()){
                        relations[i].tuples.insert(tuple);
                        extended = true;
                    }
                }
            }
        }
    }

    return make_tuple(relations, extended);
}

vector<LiftedOperatorId> FactLayerGenerator::get_applicable_actions(const ActionSchema &action, const vector<Relation> relations){
    vector<LiftedOperatorId> applicable;

    if (action.is_ground()) {
        if (is_ground_action_applicable(action, relations)){
            applicable.emplace_back(action.get_index(), vector<int>());
        }
        return applicable;
    }

    Table instantiations = instantiate(action, relations);
    if (instantiations.tuples.empty()){
        return applicable;
    }

    vector<int> free_var_indices;
    vector<int> map_indices_to_position;
    compute_map_indices_to_table_positions(
        instantiations, free_var_indices, map_indices_to_position
    );

    for (const vector<int> &tuple_with_const: instantiations.tuples){
        vector<int> ordered_tuple(free_var_indices.size());
        order_tuple_by_free_variable_order(free_var_indices,
                                           map_indices_to_position,
                                           tuple_with_const,
                                           ordered_tuple);
        applicable.emplace_back(action.get_index(), move(ordered_tuple));
    }
    return applicable;
}


bool FactLayerGenerator::is_ground_action_applicable(const ActionSchema &action, const vector<Relation> relations) const{
    for (const Atom &precond : action.get_precondition()){
        int index = precond.get_predicate_symbol_idx();
        vector<int> tuple;
        tuple.reserve(precond.get_arguments().size());
        for (const Argument &arg : precond.get_arguments()){
            assert(arg.is_constant());
            tuple.push_back(arg.get_index());
        }
        const auto& tuples_in_relation = relations[index].tuples;
        const auto& it_end_tuples_in_relation = tuples_in_relation.end();
        const auto& static_tuples = static_information.get_tuples_of_relation(index);
        const auto& it_end_static_tuples = static_tuples.end();
        if (!tuples_in_relation.empty()){
            if (precond.is_negated()){
                if (tuples_in_relation.find(tuple) != it_end_tuples_in_relation){
                    return false;
                }
            }else{
                if (tuples_in_relation.find(tuple) == it_end_tuples_in_relation){
                    return false;
                } 
            }
        }
        else if (!static_tuples.empty()){
            if (precond.is_negated()) {
                if (precond.is_negated()){
                    if (static_tuples.find(tuple) != it_end_static_tuples){
                        return false;
                    }
                }
            } else {
                if (static_tuples.find(tuple) == it_end_static_tuples){
                    return false;
                }
            }
        }
        return false;
    }
    return true;
}

void FactLayerGenerator::select_tuples(const vector<Relation> relations,
                                       const Atom &a,
                                       vector<GroundAtom> &tuples,
                                       const vector<int> &constants){
    for (const GroundAtom &atom : relations[a.get_predicate_symbol_idx()].tuples){
        bool match_constants = true;
        for (int c : constants){
            assert(a.get_arguments()[c].is_constant());
            if (atom[c] != a.get_arguments()[c].get_index()){
                match_constants = false;
                break;
            }
        }
        if (match_constants) tuples.push_back(atom);
    }
}

bool FactLayerGenerator::parse_precond_into_join_program(const PrecompiledActionData &adata,
                                                         const vector<Relation> relations,
                                                         vector<Table>& tables){
    /*
     * Parse the state and the atom preconditions into a set of tables
     * to perform the join-program more easily.
     *
     * We first obtain all indices in the precondition that are constants.
     * Then, we create the table applying the projection over the arguments
     * that satisfy the instantiation of the constants. There are two cases
     * for the projection:
     *    1. The table comes from the static information; or
     *    2. The table comes directly from the current state.
     *
     */
    if (adata.statically_inapplicable) return false;

    tables = adata.precompiled_db;
    for (unsigned i:adata.fluent_tables) {
        const Atom &atom = adata.relevant_precondition_atoms[i];
        assert(!is_static(atom.get_predicate_symbol_idx()));

        vector<GroundAtom> tuples;
        vector<int> constants, indices;

        get_indices_and_constants_in_preconditions(indices, constants, atom);
        select_tuples(relations, atom, tuples, constants);

        if (tuples.empty()) return false;

        tables[i] = Table(move(tuples), move(indices));
    }
    return true;
}

Table FactLayerGenerator::instantiate(const ActionSchema &action, const vector<Relation> relations){
    if (action.is_ground()){
        throw runtime_error("Shouldn't be calling instantiate() on a ground action");
    }

    const auto& actiondata = action_data[action.get_index()];

    vector<Table> tables(0);
    auto res = parse_precond_into_join_program(actiondata, relations, tables);

    if (!res) return Table::EMPTY_TABLE();

    assert(!tables.empty());
    assert(tables.size() == actiondata.relevant_precondition_atoms.size());

    Table &working_table = tables[0];
    for (size_t i = 1; i < tables.size(); i++){
        hash_join(working_table, tables[i]);
        filter_static(action, working_table);
        if (working_table.tuples.empty()){
            return working_table;
        }
    }

    return working_table;
}
