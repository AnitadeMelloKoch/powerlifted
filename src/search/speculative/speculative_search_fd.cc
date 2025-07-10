#include <mpi.h>
#include <memory>

#include "speculative_search_fd.h"
#include "speculative_scope.h"
#include "../writer.h"

#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

bool SpeculativeSearchFD::search(Task scoped_task){
    // need to save the pddl files (need domain and task files)
    // run fd using fast-downward.py from c++
    // need to work out how to get result from run. I think it will be in the system exit code

    // send domain file name to options
    // save pddl file
    // set up command line arguments for fd run

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    auto tmp_dir = "tmp_rank_" + to_string(rank);
    filesystem::create_directories(tmp_dir);

    auto task_filename = tmp_dir + "/task.pddl";
    scope->write(scoped_task, task_filename);
    auto sas_file_name = "output_" + to_string(rank) + ".sas";
    
    string command = "python fast-downward.py --build fd-builds/release/bin --plan-file " + opt.get_save_folder()
    + "/" + opt.get_plan_file() + "rank_" + to_string(rank)
    + " " + "--sas-file " + sas_file_name + " "
    + opt.get_domain_file() + " " + task_filename + " "
    + "--search \"" + opt.get_fd_search_opt() + "\""; 
    
    // command += " > planner_stdout.txt 2> planner_stderr.txt";
    
    int code = system(command.c_str());
    
    filesystem::remove_all(tmp_dir);
    filesystem::remove(sas_file_name);
    
    if (code == 0){
        cout << "Plan found!" << endl;
        return true;
    }

    return false;
}