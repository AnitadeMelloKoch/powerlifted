#ifndef SPECULATIVE_SEARCH
#define SPECULATIVE_SEARCH
#include "speculative_scope.h"
#include "../options.h"
#include <memory>
#include <chrono>

class SpeculativeSearch{
    protected:
        Options opt;
        int seed;
        std::unique_ptr<SpeculativeScope> scope;
        int scope_count = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        bool preserve_links;
        
        virtual bool search(Task scoped_task) = 0;
        int run_validate(const std::string& command);
    
    public:
        int speculative_search(int argc, char *argv[]);

        SpeculativeSearch(const Task &task, 
                          Options &opt,
                          int seed=42,
                          int max_attempts=500);
        
        virtual ~SpeculativeSearch();
};


#endif