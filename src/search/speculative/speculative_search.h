#ifndef SPECULATIVE_SEARCH
#define SPECULATIVE_SEARCH
#include "speculative_scope.h"
#include "../options.h"

class SpeculativeSearch{
    protected:
        SpeculativeScope scope;
        Options opt;
        int seed;
        virtual bool search(Task scoped_task) = 0;
    
    public:
        int speculative_search(int argc, char *argv[]);

        SpeculativeSearch(const Task &task, 
                          Options &opt,
                          int seed=42,
                          int max_attempts=500);
        
        virtual ~SpeculativeSearch();
};


#endif