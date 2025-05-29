#include <memory>
#include <string>
#include <queue>

#include "speculative_search_power.h"
#include "speculative_scope.h"
#include "../heuristics/heuristic_factory.h"
#include "../heuristics/heuristic.h"
#include "../successor_generators/successor_generator_factory.h"
#include "../successor_generators/successor_generator.h"
#include "../search_engines/search_factory.h"
#include "../search_engines/search.h"

using namespace std;

bool SpeculativeSearchPower::search(Task scoped_task){
    unique_ptr<Heuristic> heuristic(HeuristicFactory::create(opt, scoped_task));
    unique_ptr<SuccessorGenerator> sgen(SuccessorGeneratorFactory::create(opt.get_successor_generator(),
                                                                          opt.get_seed(),
                                                                          scoped_task));

    auto exitcode = searcher->search(scoped_task, *sgen, *heuristic);

    int code = static_cast<int>(exitcode);

    if (code == 0){
        searcher->print_statistics();
        return true;
    }

    return false;
}