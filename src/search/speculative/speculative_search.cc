#include <mpi.h>
#include <memory>
#include <string>
#include <queue>

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

    // queue of tasks -> initialization

    int work_request_tag = 0;
    int result_tag = 1;
    int list_size_tag = 3;
    int scope_tag = 4;
    vector<int> success_code(world_size-1, -1);

    if (rank == 0){
        // rank 0 gets all scopes we want to try
        bool task_complete = false;
        queue<vector<int>> task_queue;
        vector<MPI_Request> work_requests(world_size-1);
        vector<MPI_Request> result_requests(world_size-1);
        vector<int> work_flags(world_size-1, 0);
        vector<int> result_flags(world_size-1, 0);
        MPI_Request bcast_request;
        

        for (int i = 1; i < world_size; ++i){
            auto obj_list = scope.sample_scope();
            if (obj_list.size() > 0){
                task_queue.push(obj_list);
            }
        }

        // receive requests from workers
        for (int i = 1; i < world_size; ++i){
            MPI_Irecv(nullptr, 0, MPI_INT, i, work_request_tag, MPI_COMM_WORLD, &work_requests[i-1]);
        }

        for (int i = 1; i < world_size; ++i){
            MPI_Irecv(&success_code[i-1], 1, MPI_INT, i, result_tag, MPI_COMM_WORLD, &result_requests[i-1]);
        }

        // populate task queue
        // receive task requests -> tag id 0
        // when get recieve send
        // recieve that result -> tag id 1
        // update accordingly
        // if result is success send success sig -> tag id 2
        // if queue empty send kill sig -> tag id 2
        // while this happening make new scopes

        while (!task_complete){
            auto obj_list = scope.sample_scope();
            if (obj_list.size() > 0){
                task_queue.push(obj_list);
            }

            for (int i = 0; i < world_size-1; ++i){
                MPI_Test(&work_requests[i], &work_flags[i], MPI_STATUS_IGNORE);
                if (work_flags[i] == 1){
                    if (task_queue.size() == 0){
                        task_complete = true;
                        // MPI_Ibcast(&task_complete, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD, &bcast_request);
                        MPI_Abort(MPI_COMM_WORLD, 0);
                        break;
                    }
                    auto objects = task_queue.front();
                    task_queue.pop();
                    int list_size = objects.size();
                    MPI_Send(&list_size, 1, MPI_INT, i+1, list_size_tag, MPI_COMM_WORLD);
                    MPI_Send(objects.data(), list_size, MPI_INT, i+1, scope_tag, MPI_COMM_WORLD);
                    MPI_Irecv(nullptr, 0, MPI_INT, i+1, work_request_tag, MPI_COMM_WORLD, &work_requests[i]);
                }

                MPI_Test(&result_requests[i], &result_flags[i], MPI_STATUSES_IGNORE);
                if (result_flags[i] == 1){
                    if (success_code[i] == 0){
                        task_complete = true;
                        // MPI_Ibcast(&task_complete, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD, &bcast_request);
                        MPI_Abort(MPI_COMM_WORLD, 0);
                        break;
                    }
                    MPI_Irecv(&success_code[i], 1, MPI_INT, i+1, result_tag, MPI_COMM_WORLD, &result_requests[i]);
                }
            }
        }

        MPI_Wait(&bcast_request, MPI_STATUS_IGNORE);
        
    } else {
        // receive scope from rank 0 and speculatively plan
        // first need to receive vector size before can receive vector

        // request a scope -> blocking
        // plan for scope
        // send result -> blocking
        // check if finished -> non-blocking

        bool search_complete = false;

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
        
        // MPI_Request bcast_request;
        // MPI_Irecv(&search_complete, 1, MPI_INT, 0, MPI_COMM_WORLD, &bcast_request);

        while (!search_complete){
            MPI_Send(nullptr, 0, MPI_INT, 0, work_request_tag, MPI_COMM_WORLD);
            int list_size;
            if (search_complete){
                break;
            }
            cout << search_complete << endl;
            MPI_Recv(&list_size, 1, MPI_INT, 0, list_size_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> obj_list = vector<int>(list_size, 0);
            if (search_complete){
                break;
            }
            cout << search_complete << endl;
            MPI_Recv(obj_list.data(), list_size, MPI_INT, 0, scope_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

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
            if (search_complete){
                break;
            }
            cout << search_complete << endl;
            MPI_Send(&code, 1, MPI_INT, 0, result_tag, MPI_COMM_WORLD);
        }
        // MPI_Wait(&bcast_request, MPI_STATUS_IGNORE);
    }


    MPI_Finalize();

    // should change this to what I should actually return
    return 0;

}
