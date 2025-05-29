#ifndef SPECULATIVE_SEARCH_FD
#define SPECULATIVE_SEARCH_FD
#include "speculative_scope.h"
#include "speculative_search.h"
#include "../options.h"

class SpeculativeSearchFD : public SpeculativeSearch{
    protected:
        bool search(Task scoped_task) override;
    public:
        SpeculativeSearchFD(const Task &task,
                            Options opt,
                            int seed=42,
                            int max_attempts=500) : SpeculativeSearch(task,
                                                                      opt,
                                                                      seed,
                                                                      max_attempts) {}

        ~SpeculativeSearchFD() override {};

};

#endif