#ifndef SPECULATIVE_SEARCH_POWER
#define SPECULATIVE_SEARCH_POWER
#include "speculative_scope.h"
#include "speculative_search.h"
#include "../options.h"
#include "../search_engines/search.h"
#include "../search_engines/search_factory.h"

#include <memory>

class SpeculativeSearchPower : public SpeculativeSearch{
    private:
        std::unique_ptr<SearchBase> searcher;
    protected:
        bool search(Task scoped_task) override;
    public:
        SpeculativeSearchPower(const Task &task, 
                               Options opt, 
                               int seed = 42, 
                               int max_attempts = 500) : SpeculativeSearch(task,
                                                                           opt,
                                                                           seed,
                                                                           max_attempts) 
            {
                searcher = std::unique_ptr<SearchBase>(SearchFactory::create(opt, opt.get_search_engine(), opt.get_state_representation()));
            }

        ~SpeculativeSearchPower() override {};
};


#endif