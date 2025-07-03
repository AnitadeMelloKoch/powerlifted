#include "writer.h"
#include "task.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std;

Writer::Writer(string domain_file){
    ifstream file(domain_file);

    string line;
    bool in_section = false;

    if (!file){
        cerr << "Error opening file: " << domain_file << endl; 
    }

    while (getline(file, line)){
        istringstream stream(line);
        string word;

        bool collecting_objects = false;
        while (stream >> word){
            bool obj_sec = (word.find(":objects") != string::npos);
            bool con_sec = (word.find(":constant") != string::npos);
            if (obj_sec || con_sec){
                in_section = true;
                collecting_objects = true;
                continue;
            }

            if (word.find(":") != string::npos){
                // in new section
                in_section = false;
                continue;
            }

            if (word == "-"){
                collecting_objects = false;
                continue;
            }

            if (collecting_objects && in_section){
                domain_defined_objects.push_back(word);
            }
        }
    }

}

bool Writer::write(Task &task, string filename){

    ofstream out(filename);
    if (!out.is_open()){
        cout << "Error: could not open file for writing" << endl;
        return false;
    }

    out << "(define (problem " << task.get_task_name() << ")" << endl;

    out << "(:domain " << task.get_domain_name() << ")" << endl;

    out << "(:objects" << endl;
    for (auto obj : task.objects){
        if (find(domain_defined_objects.begin(), domain_defined_objects.end(), obj.get_name())
            == domain_defined_objects.end())
        {
            auto types = obj.get_types();
            int type = *max_element(types.begin(), types.end());
            out << "\t" << obj.get_name() << " - " << task.type_names[type] << endl;
        }
    }
    out << ")" << endl;

    out << "(:INIT " << endl;
    auto predicates = task.predicates;
    auto objects = task.objects;

    const auto& nullary_atoms = task.initial_state.get_nullary_atoms();
    for (size_t j = 0; j < nullary_atoms.size(); ++j){
        if (nullary_atoms[j]){
            out << "\t(" << predicates[j].get_name() << ")" << endl;
        }
    }
    const auto& static_nullary_atoms = task.static_info.get_nullary_atoms();
    for (size_t j = 0; j < nullary_atoms.size(); ++j){
        if (static_nullary_atoms[j]){
            out << "\t(" << predicates[j].get_name() << ")" << endl;
        }
    }

    for (auto relation : task.initial_state.get_relations()){
        for (auto tuple : relation.tuples){
            out << "\t(" << predicates[relation.predicate_symbol].get_name() << " ";
            for (auto obj : tuple){
                out << objects[obj].get_name() << " ";
            }
            out << ")" << endl;
        }
    }
    for (auto relation : task.static_info.get_relations()){
        for (auto tuple : relation.tuples){
            auto& name = predicates[relation.predicate_symbol].get_name();
            if (name == "=" || (name.find("type@") != string::npos)){
                continue;
            }
            out << "\t(" << predicates[relation.predicate_symbol].get_name() << " ";
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


