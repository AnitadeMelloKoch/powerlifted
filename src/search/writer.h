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
    
    public:
        Writer(std::string domain_file);

        bool write(Task &task, std::string filename);

        bool write_summary(std::string filename, 
                           int scope_num, 
                           bool task_success,
                           std::chrono::time_point<std::chrono::high_resolution_clock> start, 
                           std::chrono::time_point<std::chrono::high_resolution_clock> end);
};

#endif // SEARCH_PARSER_H


