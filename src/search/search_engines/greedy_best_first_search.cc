#include "greedy_best_first_search.h"

#include "../action.h"
#include "../heuristics/heuristic.h"
#include "../states/extensional_states.h"
#include "../states/sparse_states.h"
#include "../successor_generators/successor_generator.h"
#include "search.h"
#include "utils.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <vector>

using namespace std;

template <class PackedStateT>
int GreedyBestFirstSearch<PackedStateT>::search(const Task &task,
                                                SuccessorGenerator &generator,
                                                Heuristic &heuristic)
{
    cout << "Starting greedy best first search" << endl;
    clock_t timer_start = clock();
    SparseStatePacker state_packer(task);


    priority_queue<Node, vector<Node>, NodeComparison> q;  // Queue has Node structures
    segmented_vector::SegmentedVector<pair<int, LiftedOperatorId>> cheapest_parent;

    segmented_vector::SegmentedVector<SparsePackedState> index_to_state;
    unordered_map<SparsePackedState, int, PackedStateHash> visited;

    segmented_vector::SegmentedVector<int> shortest_distance;

    index_to_state.push_back(state_packer.pack(task.initial_state));
    cheapest_parent.push_back(make_pair(-1, LiftedOperatorId(-1, vector<int>())));

    this->heuristic_layer = heuristic.compute_heuristic(task.initial_state, task) + 1;
    cout << "Initial heuristic value " << this->heuristic_layer << endl;
    statistics.report_f_value_progress(this->heuristic_layer);

    q.emplace(0, heuristic.compute_heuristic(task.initial_state, task), this->state_counter);
    shortest_distance.push_back(0);
    visited[state_packer.pack(task.initial_state)] = this->state_counter++;

    if (task.is_goal(task.initial_state)) {
        cout << "Initial state is a goal" << endl;
        print_goal_found(generator, timer_start);
        extract_plan(
            cheapest_parent, state_packer.pack(task.initial_state), visited, index_to_state, state_packer, task);

        return SOLVED;
    }

    while (not q.empty()) {
        Node head = q.top();
        size_t next = head.id;
        int h = head.h;
        int g = head.g;
        q.pop();

        statistics.report_f_value_progress(g);
        statistics.inc_expanded();

        if (this->g_layer < g) {
            this->g_layer = g;
        }
        if (g > shortest_distance[next]) {
            continue;
        }
        if (h < this->heuristic_layer) {
            this->heuristic_layer = h;
            cout << "New heuristic value expanded: h=" << h
                 << " [this->state_counter: " << this->state_counter
                 << ", this->generations: " << this->generations
                 << ", time: " << double(clock() - timer_start) / CLOCKS_PER_SEC << "]" << '\n';
        }
        assert(index_to_state.size() >= next);
        DBState state = state_packer.unpack(index_to_state[next]);
        if (task.is_goal(state)) {
            print_goal_found(generator, timer_start);
            extract_plan(
                cheapest_parent, state_packer.pack(state), visited,index_to_state, state_packer, task);
            return SOLVED;
        }
        vector<pair<DBState, LiftedOperatorId>> successors =
            generator.generate_successors(task.actions, state, task.static_info);

        this->generations += successors.size();
        statistics.inc_generated(successors.size());

        for (const pair<DBState, LiftedOperatorId> &successor : successors) {
            const DBState &s = successor.first;
            const SparsePackedState packed = state_packer.pack(s);
            const LiftedOperatorId &a = successor.second;
            int dist = g + task.actions[a.index].get_cost();
            int new_h = heuristic.compute_heuristic(s, task);
            statistics.inc_evaluations();

            auto try_to_insert = visited.insert(make_pair(packed, this->state_counter));
            if (try_to_insert.second) {
                // Inserted for the first time in the map
                cheapest_parent.push_back(make_pair(next, a));
                q.emplace(dist, new_h, this->state_counter);
                shortest_distance.push_back(dist);
                index_to_state.push_back(packed);
                this->state_counter++;
            }
            else {
                int index = try_to_insert.first->second;
                if (dist < shortest_distance[index]) {
                    cheapest_parent[index] = make_pair(next, a);
                    q.emplace(dist, new_h, index);
                    shortest_distance[index] = dist;
                }
            }
        }
    }

    print_no_solution_found(timer_start);

    return NOT_SOLVED;
}

template <class PackedStateT>
void GreedyBestFirstSearch<PackedStateT>::print_statistics() const {
    statistics.print_detailed_statistics();
//    space.print_statistics();
}


// explicit template instantiations
template class GreedyBestFirstSearch<SparsePackedState>;
template class GreedyBestFirstSearch<ExtensionalPackedState>;