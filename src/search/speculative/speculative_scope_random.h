#ifndef SPECULATIVE_SCOPE_RANDOM
#define SPECULATIVE_SCOPE_RANDOM
#include "speculative_scope.h"

#include <vector>
#include <random>

class SpeculativeScopeRandom : public SpeculativeScope{
    private:
        int geometric_sample(int min, int max);
        std::default_random_engine generator;
        double alpha = 1.5;
    public:
        SpeculativeScopeRandom(const Task &task, int seed = 42, int max_attempts = 20, std::string domain_file = "", std::string problem_file="", bool preserve_links=true):
            SpeculativeScope(task, seed, max_attempts, domain_file, problem_file, preserve_links) {};
        std::vector<int> sample_scope() override;

        ~SpeculativeScopeRandom() override;
};


#endif