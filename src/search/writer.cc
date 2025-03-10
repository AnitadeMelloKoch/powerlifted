#include "writer.h"
#include "task.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;

bool write(Task &task, string filename){

    ofstream out(filename);
    if (!out.is_open()){
        cout << "Error: could not open file for writing" << endl;
        return false;
    }

    out << "(define (problem " << task.get_task_name() << ")" << endl;

    out << "(:domain " << task.get_domain_name() << ")" << endl;

    out << "(:objects" << endl;
    for (auto obj : task.objects){
        out << obj.get_name() << " ";
    }
    out << endl << ")" << endl;

    out << "(:INIT " << endl;
    auto predicates = task.predicates;
    auto objects = task.objects;

    const auto& nullary_atoms = task.initial_state.get_nullary_atoms();
    for (size_t j = 0; j < nullary_atoms.size(); ++j){
        if (nullary_atoms[j]){
            out << "(" << predicates[j].get_name() << ")" << endl;
        }
    }

    for (auto relation : task.initial_state.get_relations()){
        for (auto tuple : relation.tuples){
            out << "(" << predicates[relation.predicate_symbol].get_name() << " ";
            for (auto obj : tuple){
                out << objects[obj].get_name() << " ";
            }
            out << ")" << endl;
        }
    }
    out << ")" << endl;

    out << "(:goal (AND" << endl;

    for (auto goal : task.get_goal().positive_nullary_goals){
        out << "(" << predicates[goal].get_name() << ")" << endl;
    }

    for (auto goal : task.get_goal().negative_nullary_goals){
        out << "(NOT " << predicates[goal].get_name() << ")" << endl;
    }

    for (auto goal_con : task.get_goal().goal){
        if (goal_con.is_negated()){
            out << " NOT (";
        } else {
            out << " (";
        }
        out << predicates[goal_con.get_predicate_index()].get_name();
        for (auto arg_idx : goal_con.get_arguments()){
            out << " " << objects[arg_idx].get_name();
        }
        out << ")" << endl;
    }
    out << "))" << endl;

    out << ")" << endl;

    out.close();

    return true;
}


