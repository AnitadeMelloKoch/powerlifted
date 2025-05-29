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

    cout << opt.get_pddl_file() << endl;
    cout << opt.get_domain_file() << endl;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    auto tmp_dir = "tmp_rank_" + to_string(rank);
    filesystem::create_directories(tmp_dir);

    auto task_filename = tmp_dir + "/task.pddl";
    write(scoped_task, task_filename);

    
    string command = "python fast-downward.py --build fd-builds/release/bin --plan-file " + opt.get_plan_file() + "rank_" + to_string(rank)
    + " " + opt.get_domain_file() + " " + task_filename + " "
    + "--search \"" + opt.get_fd_search_opt() + "\""; 
                            
    int code = system(command.c_str());
    
    filesystem::remove_all(tmp_dir);
    
    if (code == 0){
        return true;
    }

    return false;
}