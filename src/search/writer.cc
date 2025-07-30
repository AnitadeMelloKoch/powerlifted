#include "writer.h"
#include "task.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <chrono>

using namespace std;

Writer::Writer(string domain_file, string problem_file){
    // get objects
    ifstream file(domain_file);

    if (!file){
        cerr << "Error opening domain file: " << domain_file << endl; 
    }

    string line;
    bool collecting_obj = false;
    string section = "";

    while (getline(file, line)){
        // remove comments from line
        auto comment_pos = line.find(';');
        if (comment_pos != string::npos){
            line = line.substr(0, comment_pos);
        }

        // trim white space
        line.erase(0, line.find_first_not_of("\t\r\n"));
        line.erase(line.find_last_not_of("\t\r\n") + 1);

        if (line.empty()) continue;

        if (line.find(":constant") != string::npos || line.find(":objects") != string::npos){
            collecting_obj = true;
            auto pos = line.find("(:");
            if (pos != string::npos){
                auto next_space = line.find(" ");
                if (next_space != string::npos){
                    line = line.substr(pos + next_space);
                } else {
                    line = "";
                }
            }
        } else if (line.find(":") != string::npos) {
            collecting_obj = false;
        }

        if (collecting_obj){
            section += " " + line;
        }

        if (!collecting_obj && !section.empty()){
            istringstream stream(section);
            string token;
            while (stream >> token){
                if (token == "-"){
                    string type;
                    stream >> type;
                    continue;
                }
                if (!token.empty() && token[0] != '(' && token != ")"){
                    string lowered_token = token;
                    transform(lowered_token.begin(), lowered_token.end(), lowered_token.begin(),
                                          [](unsigned char c){ return tolower(c); });
                    domain_defined_objects.push_back(lowered_token);
                }
            }
            section.clear();
        }
    }

    // metric extraction
    ifstream prob_file(problem_file);

    bool collecting_metric = false;
    int metric_paren_depth = 0;
    
    if (!prob_file){
        cerr << "Error opening problem file: " << problem_file << endl;
    }

    while (getline(prob_file, line)){

        if (line.find("=") != string::npos){
            cost_expressions.push_back(line);
        }

        // collecting metric line
        if (line.find(":metric") != string::npos){
            collecting_metric = true;
        }

        if (collecting_metric){
            metric_block += line + "\n";
            for (char c : line){
                if (c == '(') metric_paren_depth++;
                else if (c == ')') metric_paren_depth--;
            }
            if (metric_paren_depth == 0){
                collecting_metric = false;
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

    vector<string> object_names;

    for (auto obj : task.objects){
        object_names.push_back(obj.get_name());
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

    for (auto expr : cost_expressions){
        if ((expr.find("total") != string::npos) || (expr.find("other") != string::npos)){
            out << "\t" << expr << endl;
        } else {
            for (auto obj : object_names){
                if (expr.find(obj) != string::npos){
                    out << "\t" << expr << endl;
                    continue;
                }
            }
        }
    }

    const auto& nullary_atoms = task.initial_state.get_nullary_atoms();
    for (size_t j = 0; j < nullary_atoms.size(); ++j){
        if (nullary_atoms[j]){
            out << "\t(" << predicates[j].get_name() << ")" << endl;
        }
    }
    const auto& static_nullary_atoms = task.static_info.get_nullary_atoms();
    for (size_t j = 0; j < static_nullary_atoms.size(); ++j){
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

    out << metric_block << endl;

    out << ")" << endl;

    out.close();

    return true;
}


bool Writer::write_summary(string filename, 
                           int scope_num, 
                           bool task_success,
                           chrono::time_point<std::chrono::high_resolution_clock> start, 
                           chrono::time_point<std::chrono::high_resolution_clock> end){
    ofstream outfile(filename);

    if (!outfile){
        cerr << "Error opening file: " << filename << endl;
        return false;
    }

    outfile << "Speculative Scoping Summary" << endl;
    outfile << "Tested Scopes: " << scope_num << endl;
    outfile << "Plan Found: " << task_success << endl;
    auto duration = chrono::duration_cast<chrono::milliseconds>(end-start);
    outfile << "Total Time: " << duration.count() << "ms" << endl;
    auto duration_s = chrono::duration_cast<chrono::seconds>(end-start);
    outfile << "Total Time: " << duration_s.count() << "s" << endl;

    outfile.close();

    return true;
}

void findAndReplaceAll(std::string& source, const std::string& old_value, const std::string& new_value) {
    size_t pos = 0;
    while ((pos = source.find(old_value, pos)) != std::string::npos) {
        source.replace(pos, old_value.length(), new_value);
        pos += new_value.length();
    }
}

bool Writer::write_duplicate_pddl(string filename, Task &task, string prefix){
    ofstream out(filename);
    if (!out.is_open()){
        cout << "Error: could not open file for writing" << endl;
        return false;
    }

    out << "(define (problem " << task.get_task_name() << prefix << ")" << endl;
    out << "(:domain " << task.get_domain_name() << ")" << endl;
    out << "(:objects" << endl;
    
    vector<string> object_names;

    for (auto obj : task.objects){
        object_names.push_back(obj.get_name());
        auto types = obj.get_types();
        int type = *max_element(types.begin(), types.end());
        if (find(domain_defined_objects.begin(), domain_defined_objects.end(), obj.get_name()) 
                == domain_defined_objects.end()){
            out << "\t" << obj.get_name() << " - " << task.type_names[type] << endl;
            out << "\t" << prefix << "_" << obj.get_name() << " - " << task.type_names[type] << endl;
        } else {
            out << "\t" << prefix << "_" << obj.get_name() << " - " << task.type_names[type] << endl;
        }
    }
    out << ")" << endl;

    out << "(:INIT " << endl;
    auto predicates = task.predicates;
    auto objects = task.objects;

    for (auto expr : cost_expressions){
        if ((expr.find("total") != string::npos) || (expr.find("other") != string::npos)){
            out << "\t" << expr << endl;
        } else {
            out << "\t" << expr << endl;
            string dup_expr = expr;
            for (auto obj_name : object_names){
                string dup_name = prefix + "_" + obj_name;
                findAndReplaceAll(dup_expr, obj_name, dup_name); 
            }
            out << "\t" << dup_expr << endl;
        }
    }

    const auto& nullary_atoms = task.initial_state.get_nullary_atoms();
    for (size_t i = 0; i < nullary_atoms.size(); ++i){
        if (nullary_atoms[i]){
            out << "\t(" << predicates[i].get_name() << ")" << endl;
        }
    }
    const auto& static_nullary_atoms = task.static_info.get_nullary_atoms();
    for (size_t i = 0; i < static_nullary_atoms.size(); ++i){
        if (static_nullary_atoms[i]){
            out << "\t(" << predicates[i].get_name() << ")" << endl;
        }
    }

    for (auto relation : task.initial_state.get_relations()){
        for (auto tuple : relation.tuples){
            out << "\t(" << predicates[relation.predicate_symbol].get_name() << " ";
            for (auto obj : tuple){
                out << objects[obj].get_name() << " ";
            }
            out << ")" << endl;

            out << "\t(" << predicates[relation.predicate_symbol].get_name() << " ";
            for (auto obj : tuple){
                out << prefix << "_" << objects[obj].get_name() << " ";
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

            out << "\t(" << predicates[relation.predicate_symbol].get_name() << " ";
            for (auto obj : tuple){
                out << prefix << "_" << objects[obj].get_name() << " ";
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

    out << metric_block << endl;

    out << ")" << endl;

    out.close();

    return true;

}