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

#include <memory>

using namespace std;



FactLayerGenerator::FactLayerGenerator(const Task &task) 
    : GenericJoinSuccessor(task), static_info(task.static_info){
    action_data = precompile_action_data(task.get_action_schemas());
}

void FactLayerGenerator::dump_relation_list(std::vector<PtrRelation> relations){
    cout << "relations size: " << relations.size() << " " << endl;
    for(size_t i = 0; i < relations.size(); i++){
        auto tuples = relations[i].tuples;
        cout << "relation predicate: " << relations[i].predicate_symbol << " ";
        for (auto &tuple:tuples) {
            cout << "(";
            for (auto obj : *tuple){
                cout << obj << ",";
            }
            cout << "), ";
        }
        cout << " | ";
    }
    cout << endl;
}

void FactLayerGenerator::print_relation_stats(vector<PtrRelation> relations){
    cout << "relation num: " << relations.size() << endl;
    for (size_t i = 0; i < relations.size(); i++){
        cout << "relation predicate: " << relations[i].predicate_symbol << " ";
        cout << "size: " << relations[i].tuples.size() << endl;
    }
}

void FactLayerGenerator::print_relation_stats(vector<Relation> relations){
    cout << "relation num: " << relations.size() << endl;
    for (size_t i = 0; i < relations.size(); i++){
        cout << "relation predicate: " << relations[i].predicate_symbol << " ";
        cout << "size: " << relations[i].tuples.size() << endl;
    }
}

tuple<vector<PtrRelation>, bool> FactLayerGenerator::generate_next_fact_layer(
    const std::vector<ActionSchema> action_schemas,
    vector<PtrRelation> &relations,
    const GoalCondition &goal
){
    bool extended = false;
    bool goal_grounded = false;

    for (auto &action : action_schemas){
        auto applicable = get_applicable_actions(action, relations);
        for (auto &op : applicable){
            bool op_extended;
            tie(relations, op_extended) = get_new_relation(action,
                                                           op, 
                                                           relations);
            extended = extended | op_extended;
            if (op_extended){
                if (check_goal(relations, goal)){
                    goal_grounded = true;
                    break;
                }
            }
        }
    }

    bool complete = goal_grounded || !extended;

    return make_tuple(relations, complete);
}

DBState FactLayerGenerator::generate_fact_layers(const vector<ActionSchema> action_schemas, 
                                                 const DBState &state,
                                                 const GoalCondition &goal){
    
    clock_t timer_start = clock();
    vector<bool> new_nullary_atoms(state.get_nullary_atoms());
    auto relations = vector<PtrRelation>();

    for (auto relation : state.get_relations()){
        relations.push_back(
            PtrRelation(
                relation
            )
        );
    }

    bool complete = false;
    int passes = 0;
    while (!complete){
        tie(relations, complete) = generate_next_fact_layer(action_schemas, relations, goal);
        passes += 1;
        cout << "Fact layer passes: " << passes << endl;
    }

    cout << "Fact layers generated with " << passes << " passes" << endl;
    cout << "Fact layers total time: " << double(clock() - timer_start)/CLOCKS_PER_SEC << endl;
    
    auto state_relations = vector<Relation>();
    for (auto relation : relations){
        state_relations.push_back(
            Relation(
                relation
            )
        );
    }

    return DBState(move(state_relations), move(new_nullary_atoms));
}

tuple<vector<PtrRelation>, bool> FactLayerGenerator::get_new_relation(
    const ActionSchema action,
    const PtrLiftedOperatorId& op,
    vector<PtrRelation> &new_relation
){ 
    bool expanded = false;
    if (action.is_ground()){
        expanded = apply_ground_action_effects(action, new_relation);
    }
    else{
        expanded = apply_lifted_action_effects(action, op.get_instantiation(), new_relation);
    }

    return make_tuple(new_relation, expanded);
}

bool FactLayerGenerator::apply_ground_action_effects(
    const ActionSchema &action,
    vector<PtrRelation> &relation
){
    bool expanded = false;
    for (const Atom &eff : action.get_effects()){
        GroundAtom ga;
        for (const Argument &a : eff.get_arguments()){
            // create ground atom for each effect given the instantiation
            assert(a.is_constant());
            ga.push_back(a.get_index());
        }
        assert(eff.get_predicate_symbol_idx() == relation[eff.get_predicate_symbol_idx()].predicate_symbol);
        if (not eff.is_negated()){
            auto tuples_size = relation[eff.get_predicate_symbol_idx()].tuples.size();
            relation[eff.get_predicate_symbol_idx()].tuples.insert(make_shared<vector<int>>(ga));
            if (relation[eff.get_predicate_symbol_idx()].tuples.size() > tuples_size){
                expanded = true;
            }
        }
    }

    return expanded;
}

bool FactLayerGenerator::apply_lifted_action_effects(
    const ActionSchema &action,
    const shared_ptr<vector<int>> tuple,
    vector<PtrRelation> &relation
){
    bool expanded = false;
    for (const Atom &eff : action.get_effects()){
        GroundAtom ga = tuple_to_atom(tuple, eff);
        assert(eff.get_predicate_symbol_idx() == relation[eff.get_predicate_symbol_idx()].predicate_symbol);
        if (!eff.is_negated()){
            int predicate_symbol_idx = eff.get_predicate_symbol_idx();
            if (!find_tuple(relation[predicate_symbol_idx].tuples, ga)){
                auto tuples_size = relation[eff.get_predicate_symbol_idx()].tuples.size();
                relation[eff.get_predicate_symbol_idx()].tuples.insert(make_shared<GroundAtom>(ga));
                if (relation[eff.get_predicate_symbol_idx()].tuples.size() > tuples_size){
                    expanded = true;
                }
            }
        }
    }

    return expanded;
}

const GroundAtom FactLayerGenerator::tuple_to_atom(const shared_ptr<vector<int>> tuple, const Atom &eff){
    GroundAtom ground_atom;
    ground_atom.reserve(eff.get_arguments().size());
    for (auto argument : eff.get_arguments()) {
        if (!argument.is_constant()){
            ground_atom.push_back(tuple->at(argument.get_index()));
        } else {
            ground_atom.push_back(argument.get_index());
        }
    }
    assert(find(ground_atom.begin(), ground_atom.end(), -1) == ground_atom.end());

    return ground_atom;
}

vector<PtrLiftedOperatorId> FactLayerGenerator::get_applicable_actions(const ActionSchema &action, const vector<PtrRelation> &relations){
    vector<PtrLiftedOperatorId> applicable;

    if (action.is_ground()) {
        if (is_ground_action_applicable(action, relations)){
            applicable.emplace_back(action.get_index(), 
                make_shared<vector<int>>(vector<int>()));
        }
        return applicable;
    }

    PtrTable instantiations = instantiate(action, relations);
    if (instantiations.tuples.empty()){
        return applicable;
    }

    vector<int> free_var_indices;
    vector<int> map_indices_to_position;
    compute_map_indices_to_table_positions(
        instantiations, free_var_indices, map_indices_to_position
    );

    for (const shared_ptr<vector<int>> &tuple_with_const: instantiations.tuples){
        shared_ptr<vector<int>> ordered_tuple = make_shared<vector<int>>(vector<int>(free_var_indices.size()));
        order_tuple_by_free_variable_order(free_var_indices,
                                           map_indices_to_position,
                                           tuple_with_const,
                                           ordered_tuple);
        applicable.emplace_back(action.get_index(), move(ordered_tuple));
    }
    return applicable;
}

void FactLayerGenerator::order_tuple_by_free_variable_order(const vector<int> &free_var_indices,
                                                            const vector<int> &map_indices_to_position,
                                                            const shared_ptr<vector<int>> tuple_with_const,
                                                            shared_ptr<vector<int>> ordered_tuple){
    for (size_t i = 0; i < free_var_indices.size(); ++i){
        ordered_tuple->at(free_var_indices[i]) = tuple_with_const->at(map_indices_to_position[i]);
    }
}

void FactLayerGenerator::compute_map_indices_to_table_positions(const PtrTable &instantiations,
                                                                vector<int> &free_variable_indices,
                                                                vector<int> &map_indices_to_position){
    for (size_t j = 0; j < instantiations.tuple_index.size(); ++j){
        if (instantiations.index_is_variable(j)){
            free_variable_indices.push_back(instantiations.tuple_index[j]);
            map_indices_to_position.push_back(j);
        }
    }
}

bool FactLayerGenerator::is_ground_action_applicable(const ActionSchema &action, const vector<PtrRelation> &relations) const{
    for (const Atom &precond : action.get_precondition()){
        int index = precond.get_predicate_symbol_idx();
        vector<int> tuple;
        tuple.reserve(precond.get_arguments().size());
        for (const Argument &arg : precond.get_arguments()){
            assert(arg.is_constant());
            tuple.push_back(arg.get_index());
        }
        const auto& tuples_in_relation = relations[index].tuples;
        const auto& static_tuples = static_information.get_tuples_of_relation(index);
        const auto& it_end_static_tuples = static_tuples.end();
        if (!tuples_in_relation.empty()){
            if (precond.is_negated()){
                if (find_tuple(tuples_in_relation, tuple)){
                    return false;
                }
            }else{
                if (find_tuple(tuples_in_relation, tuple)){
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

bool FactLayerGenerator::find_tuple(const unordered_set<PtrTable::ptr_tuple_t, PtrTupleHash> &tuples_in_relation, const vector<int> &tuple) const {
    for (auto rel_tup : tuples_in_relation){
        if (rel_tup->size() == tuple.size()){
            bool tuple_found = true;
            for (size_t i = 0; i < tuple.size(); i++){
                if (rel_tup->at(i) != tuple[i]){
                    tuple_found = false;
                    break;
                }
            }
            if (tuple_found) return true;
        } 
    }

    return false;
}

void FactLayerGenerator::select_tuples(const vector<PtrRelation> &relations,
                                       const Atom &a,
                                       vector<shared_ptr<GroundAtom>> &tuples,
                                       const vector<int> &constants){
    for (const shared_ptr<GroundAtom> &atom : relations[a.get_predicate_symbol_idx()].tuples){
        bool match_constants = true;
        for (int c : constants){
            assert(a.get_arguments()[c].is_constant());
            if (atom->at(c) != a.get_arguments()[c].get_index()){
                match_constants = false;
                break;
            }
        }
        if (match_constants) tuples.push_back(atom);
    }
}

vector<PtrPrecompiledActionData> FactLayerGenerator::precompile_action_data(const vector<ActionSchema>& actions){
    vector<PtrPrecompiledActionData> result;
    result.reserve(actions.size());
    for (const auto &a : actions){
        result.push_back(precompile_action_data(a));
    }
    return result;
}

PtrPrecompiledActionData FactLayerGenerator::precompile_action_data(const ActionSchema& action){
    PtrPrecompiledActionData data;


    data.is_ground = action.get_parameters().empty();
    if (data.is_ground) return data;

    for (const Atom &p : action.get_precondition()){
        bool is_ineq = (p.get_name() == "=");
        if (p.is_negated() and !is_ineq){
            throw runtime_error("Actions with negated preconditions not supported yet");
        }

        if (!p.is_ground() and !is_ineq){
            data.relevant_precondition_atoms.push_back(p);
        }
    }

    assert(!data.relevant_precondition_atoms.empty());

    data.precompiled_db.resize(data.relevant_precondition_atoms.size());

    for (size_t i = 0; i < data.relevant_precondition_atoms.size(); ++i){
        const Atom &atom = data.relevant_precondition_atoms[i];

        if (!is_static(atom.get_predicate_symbol_idx())){
            data.fluent_tables.push_back(i);
            continue;
        }

        vector<shared_ptr<GroundAtom>> tuples;
        vector<int> constants, indices;

        get_indices_and_constants_in_preconditions(indices, constants, atom);

        auto static_relations = vector<PtrRelation>();
        static_relations.reserve(static_information.get_relations().size());

        for (auto &relation : static_information.get_relations()){
            static_relations.push_back(PtrRelation(relation));
        }

        select_tuples(static_relations, atom, tuples, constants);


        if (tuples.empty()){
            data.statically_inapplicable = true;
            return data;
        }

        data.precompiled_db[i] = PtrTable(move(tuples), move(indices));
    }

    return data;
}

bool FactLayerGenerator::parse_precond_into_join_program(const PtrPrecompiledActionData &adata,
                                                         const vector<PtrRelation> &relations,
                                                         vector<PtrTable>& tables){
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

        vector<shared_ptr<GroundAtom>> tuples;
        vector<int> constants, indices;

        get_indices_and_constants_in_preconditions(indices, constants, atom);
        select_tuples(relations, atom, tuples, constants);


        if (tuples.empty()) return false;

        tables[i] = PtrTable(move(tuples), move(indices));
    }
    return true;
}

PtrTable FactLayerGenerator::instantiate(const ActionSchema &action, const vector<PtrRelation> &relations){
    
    if (action.is_ground()){
        throw runtime_error("Shouldn't be calling instantiate() on a ground action");
    }

    const auto& actiondata = action_data[action.get_index()];

    vector<PtrTable> tables(0);
    auto res = parse_precond_into_join_program(actiondata, relations, tables);

    if (!res) return PtrTable::EMPTY_TABLE();

    assert(!tables.empty());
    assert(tables.size() == actiondata.relevant_precondition_atoms.size());

    PtrTable &working_table = tables[0];
    for (size_t i = 1; i < tables.size(); i++){
        ptr_hash_join(working_table, tables[i]);
        filter_static(action, working_table);
        if (working_table.tuples.empty()){
            return working_table;
        }
    }

    return working_table;
}


void FactLayerGenerator::filter_static(const ActionSchema &action, PtrTable &working_table){
    const auto& tup_idx = working_table.tuple_index;

    for (const Atom& atom : action.get_static_precondition()){
        const vector<Argument> &args = atom.get_arguments();
        bool is_equality = true;

        if (is_equality){
            assert(args.size() == 2);
            if (args[0].is_constant() && args[1].is_constant()){
                bool is_equal = (args[0].get_index() == args[1].get_index());

                if ((atom.is_negated() && is_equal)
                        || (!atom.is_negated() && !is_equal)){
                    working_table.tuples.clear();
                    return;
                }
            }else if (args[0].is_constant() || args[1].is_constant()){
                int param_idx = -1;
                int const_idx = -1;
                if (args[0].is_constant()){
                    const_idx = args[0].get_index();
                    param_idx = args[1].get_index();
                } else {
                    const_idx = args[1].get_index();
                    param_idx = args[0].get_index();
                }
                auto it = find(tup_idx.begin(), tup_idx.end(), param_idx);
                if (it != tup_idx.end()){
                    int index = distance(tup_idx.begin(), it);

                    vector<shared_ptr<vector<int>>> new_tuples;
                    for (const auto &t : working_table.tuples){
                        if ((atom.is_negated() && t->at(index) != const_idx)
                                || (!atom.is_negated() && t->at(index) != const_idx)){
                            new_tuples.push_back(t);
                        }
                    }
                    working_table.tuples = move(new_tuples);
                }
            } else {
                auto it_1 = find(tup_idx.begin(), tup_idx.end(), args[0].get_index());
                auto it_2 = find(tup_idx.begin(), tup_idx.end(), args[1].get_index());

                if (it_1 != tup_idx.end() and it_2 != tup_idx.end()){
                    int index1 = distance(tup_idx.begin(), it_1);
                    int index2 = distance(tup_idx.begin(), it_2);

                    vector<shared_ptr<vector<int>>> new_tuples;
                    for (const auto &t : working_table.tuples){
                        if ((atom.is_negated() && t->at(index1) != t->at(index2))
                                || (!atom.is_negated() && t->at(index1) == t->at(index2))){
                            new_tuples.push_back(t);
                        }
                    }
                    working_table.tuples = move(new_tuples);
                }
            }
        }
    }
}

bool FactLayerGenerator::check_goal(const vector<PtrRelation> &relations,
                                    const GoalCondition &goal){
    for (const AtomicGoal &atomic_goal : goal.goal){
        int goal_predicate = atomic_goal.get_predicate_index();
        const PtrRelation &relation_at_goal_predicate = relations[goal_predicate];

        assert(goal_predicate == relation_at_goal_predicate.predicate_symbol);

        const auto args_in_relation = find_tuple(relation_at_goal_predicate.tuples, atomic_goal.get_arguments());
        if ((!atomic_goal.is_negated() && !args_in_relation) || (atomic_goal.is_negated() && args_in_relation)){
            return false;
        }
    }
    return true;
}


