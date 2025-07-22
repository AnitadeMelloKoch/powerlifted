#ifndef SPECULATIVE_SCOPE_COST
#define SPECULATIVE_SCOPE_COST
#include "speculative_scope.h"

#include <vector>
#include <unordered_map>

class SpeculativeScopeCost : public SpeculativeScope{
    private:
        std::vector<std::vector<int>> scope_idxs;
        std::vector<int> object_num;
        // object indexes ordered by cost (ascending)
        std::vector<std::vector<std::pair<int, int>>> ordered_obj_idxs;
        

    public:
        SpeculativeScopeCost(const Task &task, int seed = 42, int max_attempts=500, std::string domain_file = "", std::string problem_file = "");
        std::vector<int> sample_scope() override;

        ~SpeculativeScopeCost() override;
};

#endif