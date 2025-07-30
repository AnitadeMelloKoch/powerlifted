#include <mpi.h>
#include <memory>

#include <chrono>
#include <iomanip>
#include <sstream>

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
    
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&time), "%Y%m%d_%H%M%S");
    auto tmp_dir = "tmp_" + ss.str() + "_rank_" + to_string(rank);
    filesystem::create_directories(tmp_dir);

    auto task_filename = tmp_dir + "/task.pddl";
    scope->write(scoped_task, task_filename);
    auto sas_file_name = "output_" +ss.str() + "_" + to_string(rank) + ".sas";
    auto plan_file_name = opt.get_save_folder() + "/" + opt.get_plan_file() + "rank_" + to_string(rank);
    
    string command = "python fast-downward.py --build fd-builds/release/bin --overall-time-limit "
    + to_string(opt.get_process_timeout()) + " --overall-memory-limit " + opt.get_process_mem_limit() 
    + " --plan-file " + plan_file_name
    + " --sas-file " + sas_file_name + " "
    + opt.get_domain_file() + " " + task_filename + " "
    + "--search \"" + opt.get_fd_search_opt() + "\""; 
        
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
    filesystem::remove(sas_file_name);

    if (code == 0){
        cout << "Plan found!" << endl;
        return true;
    }
    return false;

}



