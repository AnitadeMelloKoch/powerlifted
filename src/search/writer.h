#ifndef SEARCH_WRITER_H
#define SEARCH_WRITER_H

#include <fstream>
#include <vector>
#include <string>
#include <chrono>

class Task;

class Writer{
    private:
        std::vector<std::string> domain_defined_objects;
        std::vector<std::string> cost_expressions;
        std::string metric_block;
    
    public:
        Writer(std::string domain_file, std::string problem_file);

        bool write(Task &task, std::string filename);

        bool write_summary(std::string filename,
                           int scope_num,
                           bool task_success,
                           std::chrono::time_point<std::chrono::high_resolution_clock> start,
                           std::chrono::time_point<std::chrono::high_resolution_clock> end,
                           double total_cpu_time_s);
        
        std::vector<std::string> get_domain_defined_objects() { return domain_defined_objects; };

        bool write_duplicate_pddl(std::string filename,
                                  Task &task,
                                  std::string prefix);
};

#endif // SEARCH_PARSER_H


