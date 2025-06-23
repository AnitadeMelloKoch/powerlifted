#ifndef SEARCH_WRITER_H
#define SEARCH_WRITER_H

#include <fstream>
#include <vector>
#include <string>

class Task;

class Writer{
    private:
        std::vector<std::string> domain_defined_objects;
    
    public:
        Writer(std::string domain_file);

        bool write(Task &task, std::string filename);
};

#endif // SEARCH_PARSER_H


