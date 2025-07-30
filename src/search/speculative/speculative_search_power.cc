#include <memory>
#include <string>
#include <queue>
#include <mpi.h>
#include <filesystem>

#include <chrono>
#include <iomanip>
#include <sstream>

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

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%Y%m%d_%H%M%S");
    auto tmp_dir = "tmp_" + ss.str() + "_rank_" + to_string(rank);
    filesystem::create_directories(tmp_dir);

    auto task_filename = tmp_dir + "/task.pddl";
    auto translater_filename = "output_" + ss.str() + "_" + to_string(rank) + ".lifted";
    scope->write(scoped_task, task_filename);
    auto plan_file_name = opt.get_save_folder() + "/" + opt.get_plan_file() + "rank_" + to_string(rank);

    string command = "python powerlifted.py -d " + opt.get_domain_file() + " -i "
                        + task_filename + " --translator-output-file " + translater_filename
                        + " --plan-file " + plan_file_name
                        + " --stop-after-first-plan --time-limit " + to_string(opt.get_process_timeout());

    int code = system(command.c_str());

    if (!preserve_links) {
        string val_command = "Validate " + opt.get_domain_file() + " "
        + opt.get_problem_file() + " " + plan_file_name;
        int val_code = run_validate(val_command);
        code = val_code;

        if (val_code != 0){
            filesystem::remove(plan_file_name);
        }   
    }

    filesystem::remove_all(tmp_dir);

    if (code == 0){
        cout << "Plan found!" << endl;
        return true;
    }

    return false;
}