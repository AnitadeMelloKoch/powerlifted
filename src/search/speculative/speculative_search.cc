#include "speculative_search.h"
#include "speculative_scope.h"
#include "speculative_scope_cost.h"
#include "speculative_scope_random.h"
#include "../plan_manager.h"
#include "../writer.h"

#include <mpi.h>
#include <memory>
#include <string>
#include <queue>
#include <vector>
#include <chrono>

using namespace std;

SpeculativeSearch::SpeculativeSearch(const Task &task, Options &opt, int seed, int max_attempts)
    :opt(opt), seed(seed) {
    start = chrono::high_resolution_clock::now();
    auto scoping_method = opt.get_scoping_method();
    if (scoping_method == "cost"){
        scope = make_unique<SpeculativeScopeCost>(task, seed, max_attempts, opt.get_domain_file(), opt.get_problem_file());
    } else if (scoping_method == "random"){
        scope = make_unique<SpeculativeScopeRandom>(task, seed, max_attempts, opt.get_domain_file(), opt.get_problem_file());
    } else {
        cout << "No valid scoping method provided" << endl;
    }
}

int SpeculativeSearch::speculative_search(int argc, char *argv[]){
    scope->dump_stats(scope->get_task());
    
    int rank, world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int work_request_tag = 0;
    int result_tag = 1;
    int list_size_tag = 3;
    int scope_tag = 4;
    int end_signal_tag = 5;
    vector<int> successes(world_size - 1, 0);
    vector<bool> end_signal_received(world_size - 1, false);
    bool task_success = false;

    if (rank == 0){
        // rank 0 creates all scopes and waits for results
        bool task_complete = false;
        queue<vector<int>> task_queue;

        vector<MPI_Request> work_requests(world_size - 1);
        for (size_t i = 0; i < work_requests.size(); ++i) {
            work_requests[i] = MPI_REQUEST_NULL;  
        }

        vector<MPI_Request> result_requests(world_size - 1);
        for (size_t i = 0; i < result_requests.size(); ++i) {
            result_requests[i] = MPI_REQUEST_NULL;  
        }

        vector<MPI_Request> end_signal_requests(world_size - 1);
        for (size_t i = 0; i < end_signal_requests.size(); ++i){
            end_signal_requests[i] = MPI_REQUEST_NULL;
        }

        vector<int> work_flags(world_size - 1, 0);
        vector<int> result_flags(world_size - 1, 0);
        vector<int> end_signal_flags(world_size - 1, 0);

        // add some initial scopes 
        for (int i = 1; i < 2*world_size; ++i){
            auto obj_list = scope->sample_scope();
            if (obj_list.size() > 0){
                task_queue.push(obj_list);
            }
        }

        //receive requests from workers
        for (int i = 1; i < world_size; ++i){
            MPI_Irecv(nullptr,
                      0,
                      MPI_INT,
                      i,
                      work_request_tag,
                      MPI_COMM_WORLD,
                      &work_requests[i-1]);
        }
        // receive results from complete searches
        for (int i = 1; i < world_size; ++i){
            MPI_Irecv(&successes[i-1],
                      1,
                      MPI_INT,
                      i,
                      result_tag,
                      MPI_COMM_WORLD,
                      &result_requests[i-1]);
        }
        // recieve note of end signal received
        for (int i = 1; i < world_size; ++i){
            MPI_Irecv(nullptr,
                      0,
                      MPI_INT,
                      i,
                      end_signal_tag,
                      MPI_COMM_WORLD,
                      &end_signal_requests[i-1]);
        }

        while (true){
            // add a new scope to task queue
            auto obj_list = scope->sample_scope();
            if (obj_list.size() > 0){
                task_queue.push(obj_list);
            }

            for (int i = 0; i < world_size - 1; ++i){
                // send new scope to ranks that request 
                MPI_Test(&work_requests[i], &work_flags[i], MPI_STATUS_IGNORE);
                if (work_flags[i] == 1){
                    // we have no more scopes left.
                    // search ended unsuccessfully
                    if (task_queue.size() == 0){
                        task_complete = true;
                    }
                    if (task_complete){
                        int end_signal_list_size = -1;
                        MPI_Send(&end_signal_list_size, 1, MPI_INT, i+1, list_size_tag, MPI_COMM_WORLD);
                    } else{
                        auto objects = task_queue.front();
                        task_queue.pop();
                        int list_size = objects.size();
                        MPI_Send(&list_size, 1, MPI_INT, i+1, list_size_tag, MPI_COMM_WORLD);
                        MPI_Send(objects.data(), list_size, MPI_INT, i+1, scope_tag, MPI_COMM_WORLD);
                        MPI_Irecv(nullptr, 0, MPI_INT, i+1, work_request_tag, MPI_COMM_WORLD, &work_requests[i]);
                    }
                }
                // check results from completed tasks
                MPI_Test(&result_requests[i],  &result_flags[i], MPI_STATUSES_IGNORE);
                if (result_flags[i] == 1){
                    scope_count += 1;
                    if (successes[i] == 1){
                        task_complete = true;
                        task_success = true;
                        cout << "task success after " << scope_count << " scopes" << endl;
                    }
                    MPI_Irecv(&successes[i], 1, MPI_INT, i+1, result_tag, MPI_COMM_WORLD, &result_requests[i]);
                }
                // check for end signal requests
                MPI_Test(&end_signal_requests[i], &end_signal_flags[i], MPI_STATUSES_IGNORE);
                if (end_signal_flags[i] == 1){
                    end_signal_received[i] = true;
                }
            }
            if (all_of(end_signal_received.begin(), end_signal_received.end(), [](bool b) { return b; })){
                break;
            }
        }
    } else {
        string plan_name = opt.get_save_folder() + "/" + opt.get_plan_file() + "_rank" + to_string(rank);
        PlanManager::set_plan_filename(plan_name);
        string pddl_name = opt.get_save_folder() + "/" + opt.get_pddl_file();
        auto pos_dot = pddl_name.find('.');
        if (pos_dot != string::npos){
            pddl_name = pddl_name.substr(0, pos_dot);
        }
        pddl_name = pddl_name + "_rank" + to_string(rank) + ".pddl";
        PlanManager::set_pddl_filename(pddl_name);

        while (true){
            MPI_Send(nullptr, 0, MPI_INT, 0, work_request_tag, MPI_COMM_WORLD);
            int list_size = 0;
            MPI_Recv(&list_size, 1, MPI_INT, 0, list_size_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (list_size < 0){
                MPI_Send(nullptr, 0, MPI_INT, 0, end_signal_tag, MPI_COMM_WORLD);
                break;
            }

            vector<int> obj_list = vector<int>(list_size, 0);
            MPI_Recv(obj_list.data(), list_size, MPI_INT, 0, scope_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // cout << "rank[" << rank << "] Scope received: ";
            // for (auto idx : obj_list){
            //     cout << idx << " ";
            // }
            // cout << endl;

            auto scoped_task = scope->speculative_scope(obj_list);

            bool success = search(scoped_task);
            
            int success_int = success;
            MPI_Send(&success_int, 1, MPI_INT, 0, result_tag, MPI_COMM_WORLD);            

            scope->write(scoped_task, PlanManager::get_pddl_filename());

            if (success){
                scope->write(scoped_task, PlanManager::get_pddl_filename());
                MPI_Send(nullptr, 0, MPI_INT, 0, end_signal_tag, MPI_COMM_WORLD);
                break;
            }
        }
    }

    if (rank == 0){
        auto end = chrono::high_resolution_clock::now();
        scope->write_summary(opt.get_save_folder() + "/" + "summary.out", scope_count, task_success, start, end);
    }

    MPI_Finalize();

    

    return 0;

}

SpeculativeSearch::~SpeculativeSearch() {}