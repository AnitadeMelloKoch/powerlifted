#include <mpi.h>
#include <memory>
#include <string>

#include "speculative_search.h"
#include "speculative_scope.h"
#include "../heuristics/heuristic_factory.h"
#include "../heuristics/heuristic.h"
#include "../successor_generators/successor_generator_factory.h"
#include "../successor_generators/successor_generator.h"
#include "../search_engines/search_factory.h"
#include "../search_engines/search.h"
#include "../plan_manager.h"
#include "../writer.h"

using namespace std;

SpeculativeSearch::SpeculativeSearch(const Task &task,  Options opt, int seed, int max_attempts)
    :scope(SpeculativeScope(task, seed, max_attempts)), opt(opt){
    
}

int SpeculativeSearch::speculative_search(int argc, char *argv[]){
    int rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0){
        // rank 0 gets all scopes we want to try
        bool search_success = false;
        vector<int> exit_codes(world_size-1, -1);

        while (!search_success){
            for (int i = 1; i < world_size; i++){
                auto obj_list = scope.sample_scope();
                if (obj_list.size() == 0){
                    cout << "object list is empty. Currently throws error. Don't worry about it!" << endl;
                    return -1;
                }
                int list_size = obj_list.size();
                // to send the vector we first need to send the vector size 
                // before sending the vector
                MPI_Send(&list_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

                MPI_Send(obj_list.data(), list_size, MPI_INT, i, 1, MPI_COMM_WORLD);
            }

            for (int i = 0; i < (world_size-1); i++){
                int exit_code;
                MPI_Recv(&exit_code, 1, MPI_INT, i+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                exit_codes[i] = exit_code;
            }

            if (find(exit_codes.begin(), exit_codes.end(), 0) != exit_codes.end()){
                search_success = true;
            }

            for (int i = 1; i < world_size; i++){
                int success_int = int(search_success);
                MPI_Send(&success_int, 1, MPI_INT, i, 3, MPI_COMM_WORLD);
            }
        }
        
    } else {
        // receive scope from rank 0 and speculatively plan
        // first need to receive vector size before can receive vector
        bool search_success = false;

        unique_ptr<SearchBase> search(SearchFactory::create(opt, opt.get_search_engine(), opt.get_state_representation()));
        
        string plan_name = opt.get_plan_file() + "_rank" + to_string(rank);
        PlanManager::set_plan_filename(plan_name);
        string pddl_name = opt.get_pddl_file();
        auto pos_dot = pddl_name.find('.');
        if (pos_dot != string::npos){
            pddl_name = pddl_name.substr(0, pos_dot);
        }
        pddl_name = pddl_name + "_rank" + to_string(rank) + ".pddl";
        PlanManager::set_pddl_filename(pddl_name);

        while (!search_success){
            int list_size;
            MPI_Recv(&list_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> obj_list = vector<int>(list_size, 0);
            MPI_Recv(obj_list.data(), list_size, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            auto scoped_task = scope.speculative_scope(obj_list);

            scope.dump_stats(scoped_task);

            unique_ptr<Heuristic> heuristic(HeuristicFactory::create(opt, scoped_task));
            unique_ptr<SuccessorGenerator> sgen(SuccessorGeneratorFactory::create(opt.get_successor_generator(),
                                                                                  opt.get_seed(),
                                                                                  scoped_task));

            auto exitcode = search->search(scoped_task, *sgen, *heuristic);
            int code = static_cast<int>(exitcode);
            cout << "SUCCESS CODE " << code << endl;

            if(code==0){
                search->print_statistics();
                write(scoped_task, PlanManager::get_pddl_filename());
            }

            MPI_Send(&code, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            int search_success_int;
            MPI_Recv(&search_success_int, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            search_success = bool(search_success_int);
        }
    }

    MPI_Finalize();

    // should change this to what I should actually return
    return 0;

}
