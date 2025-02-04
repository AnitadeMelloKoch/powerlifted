#ifndef SPECULATIVE_SEARCH
#define SPECULATIVE_SEARCH
#include "speculative_scope.h"
#include "../options.h"

class SpeculativeSearch{
    private:
        SpeculativeScope scope;
        int seed;
        Options opt;

    public:
        int speculative_search(int argc, char *argv[]);

        explicit SpeculativeSearch(const Task &task, Options opt, int seed = 42, int max_attempts = 500);
};


#endif