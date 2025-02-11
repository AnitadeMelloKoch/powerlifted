#include "options.h"
#include "parser.h"
#include "plan_manager.h"
#include "task.h"

#include "heuristics/heuristic.h"
#include "heuristics/heuristic_factory.h"
#include "search_engines/search.h"
#include "search_engines/search_factory.h"
#include "successor_generators/successor_generator.h"
#include "successor_generators/successor_generator_factory.h"
#include "fact_layer/fact_layer_generator.h"

// TODO This should be included in the heuristic, not here. Right now it is here for testing
#include "heuristics/ff_heuristic.h"

#include <iostream>
#include <memory>

using namespace std;
using namespace utils;

int main(int argc, char *argv[]) {
    cout << "Initializing planner" << endl;

    Options opt(argc, argv);

    ifstream task_file(opt.get_filename());
    if (!task_file) {
        cerr << "Error opening the task file: " << opt.get_filename() << endl;
        exit(-1);
    }

    cout << "Reading task description file." << endl;
    cin.rdbuf(task_file.rdbuf());

    // Parse file
    string domain_name, task_name;
    cin >> domain_name >> task_name;
    Task task(domain_name, task_name);
    cout << task.get_domain_name() << " " << task.get_task_name() << endl;

    bool parsed = parse(task, task_file);
    if (!parsed) {
        cerr << "Parser failed." << endl;
        exit(-1);
    }

    cout << "IMPORTANT: Assuming that negative effects are always listed first. "
            "(This is guaranteed by the default translator.)" << endl;

    // Let's create a couple unique_ptr's that deal with mem allocation themselves
    std::unique_ptr<SearchBase> search(SearchFactory::create(opt, opt.get_search_engine(), opt.get_state_representation()));
    std::unique_ptr<Heuristic> heuristic(HeuristicFactory::create(opt, task));
    std::unique_ptr<SuccessorGenerator> sgen(SuccessorGeneratorFactory::create(opt.get_successor_generator(),
                                                                               opt.get_seed(),
                                                                               task));
    
    std::unique_ptr<FactLayerGenerator> forward_reachability = make_unique<FactLayerGenerator>(task);

    PlanManager::set_plan_filename(opt.get_plan_file());

    // Start search
    if (task.is_trivially_unsolvable()) {
        cout << "Problem goal was statically determined to be unsatisfiable." << endl;
        exit_with(utils::ExitCode::SEARCH_UNSOLVABLE);
    }

    try {
        if (opt.get_forward_reachability()){
            auto new_state = forward_reachability->generate_fact_layers(task.get_action_schemas(),
                                                                        task.initial_state,
                                                                        task.get_goal());
            task.dump_state(new_state);
        }else{
            auto exitcode = search->search(task, *sgen, *heuristic);
            search->print_statistics();
            utils::report_exit_code_reentrant(exitcode);
            return static_cast<int>(exitcode);
        }
        
    }
    // catch (const bad_alloc& ex) {
    //     //search->print_statistics();
    //     exit_with(utils::ExitCode::SEARCH_OUT_OF_MEMORY);
    // }
    catch (const exception& ex) {
        //search->print_statistics();
        cerr << ex.what();
    }

}
