#ifndef SPECULATIVE_SCOPE_RANDOM
#define SPECULATIVE_SCOPE_RANDOM
#include "speculative_scope.h"

#include <vector>

class SpeculativeScopeRandom : public SpeculativeScope{
    public:
        SpeculativeScopeRandom(const Task &task, int seed = 42, int max_attempts = 500):
            SpeculativeScopeRandom(task, seed, max_attempts) {};
        std::vector<int> sample_scope() override;

        ~SpeculativeScopeRandom() override;
}


#endif