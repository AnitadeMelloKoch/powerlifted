#include "hash_join.h"
#include "../hash_structures.h"
#include "table.h"
#include "utils.h"

#include <algorithm>
#include <cassert>
#include <unordered_map>

#include <iostream>

using namespace std;

std::vector<int> project_tuple(
    const std::vector<int>& tuple,
    const std::vector<int>& pattern)
{
    auto sz = pattern.size();
    vector<int> projected(sz);
    for (size_t i = 0; i < sz; ++i) {
        projected[i] = tuple[pattern[i]];
    }
    return projected;
}

vector<int> ptr_project_tuple(
    const std::vector<int> tuple,
    const std::vector<int>& pattern
){
    auto sz = pattern.size();
    vector<int> projected(sz);
    for (size_t i = 0; i < sz; ++i){
        projected[i] = tuple[pattern[i]];
    }

    return projected;
} 

void hash_join(Table &t1, const Table &t2) {
    /*
     * This function implements a hash join as follows
     *
     * 1. Start by checking which indexes have the same argument
     * 2. If there is no match, we perform a cartesian product
     * 3. Otherwise, we loop over the first table, create a hash over the
     *    matching keys. Then, loop over the second table searching for hits
     *    in the hash table.
     */
    std::vector<int> matches1, matches2;
    compute_matching_columns(t1, t2, matches1, matches2);
    assert(matches1.size()==matches2.size());

    vector<vector<int>> new_tuples;
    if (matches1.empty()) {
        /*
         * If no attribute matches, then we apply a cartesian product
         * TODO this code is duplicate from join.cc, make it an auxiliary function
         */
        t1.tuple_index.insert(t1.tuple_index.end(), t2.tuple_index.begin(), t2.tuple_index.end());
        for (const vector<int> &tuple_t1 : t1.tuples) {
            for (const vector<int> &tuple_t2 : t2.tuples) {
                vector<int> aux(tuple_t1);
                aux.insert(aux.end(), tuple_t2.begin(), tuple_t2.end());
                new_tuples.push_back(std::move(aux));
            }
        }
    }
    else {
        unordered_map<vector<int>, vector<vector<int>>, TupleHash> hash_join_map;
        // Build phase
        for (const vector<int> &tuple : t1.tuples) {
            hash_join_map[project_tuple(tuple, matches1)].push_back(tuple);
        }

        // Remove duplicated index. Duplicate code from join.cc
        vector<bool> to_remove(t2.tuple_index.size(), false);
        for (const auto &m : matches2) {
            to_remove[m] = true;
        }

        for (size_t j = 0; j < t2.tuple_index.size(); ++j) {
            if (!to_remove[j]) {
                t1.tuple_index.push_back(t2.tuple_index[j]);
            }
        }

        // Probe phase
        for (vector<int> tuple : t2.tuples) {

            auto it = hash_join_map.find(project_tuple(tuple, matches2));

            if (it != hash_join_map.end()) {
                const auto& matching_tuples = it->second;
                for (vector<int> t:matching_tuples) {
                    for (unsigned j = 0; j < to_remove.size(); ++j) {
                        if (!to_remove[j]) t.push_back(tuple[j]);
                    }
                    new_tuples.push_back(std::move(t));
                }
            }
        }

    }
    t1.tuples = std::move(new_tuples);
}

void collect_indices(const PtrTable &table, const vector<int> &indices, unordered_set<shared_ptr<vector<int>>, PtrTupleHash, PtrTupleEq> &new_tuples){
    for (shared_ptr<vector<int>> tuple : table.tuples){
        vector<int> new_tuple = vector<int>();
        for (int idx : indices){
            new_tuple.push_back(tuple->at(idx));
        }
        new_tuples.insert(make_shared<vector<int>>(new_tuple));
    }
}

void ptr_hash_join(PtrTable &t1, const PtrTable &t2, const vector<int> &relevant_args){
    std::vector<int> matches1, matches2;
    ptr_compute_matching_columns(t1, t2, matches1, matches2);
    assert(matches1.size()==matches2.size());

    std::vector<int> columns1, columns2;
    ptr_compute_relevant_columns(t1, t2, columns1, columns2, relevant_args);
    vector<int> arg_idx = vector<int>();

    if ((columns1.size() == 0) & (columns2.size() == 0)){
        return;
    }

    //////////////////////////////////////////////////////////////////////
    // cout << "relevant args" << endl;
    // for (auto idx : relevant_args){
    //     cout << "| " << idx << " | ";
    // }
    // cout << endl;

    // cout << "t1 indices" << endl;
    // for (auto idx : t1.tuple_index){
    //     cout << "| " << idx << " | ";
    // }
    // cout << endl;

    // cout << "t1 columns to keep" << endl;
    // for (auto idx : columns1){
    //     cout << "| " << idx << " | ";
    // }
    // cout << endl;

    // cout << "table 1 tuples" << endl;
    // cout << "_______________________" << endl;
    // for (auto tuple : t1.tuples){
    //     for (auto elem : *tuple){
    //         cout << "- " << elem << " - ";
    //     }
    //     cout << endl;
    // }
    // cout << "_______________________" << endl;

    // cout << "t2 indices" << endl;
    // for (auto idx : t2.tuple_index){
    //     cout << "| " << idx << " | ";
    // }
    // cout << endl;

    // cout << "t2 columns to keep" << endl;
    // for (auto idx : columns2){
    //     cout << "| " << idx << " | ";
    // }
    // cout << endl;

    // cout << "table 2 tuples" << endl;
    // cout << "_______________________" << endl;
    // for (auto tuple : t2.tuples){
    //     for (auto elem : *tuple){
    //         cout << "- " << elem << " - ";
    //     }
    //     cout << endl;
    // }
    // cout << "_______________________" << endl;
    //////////////////////////////////////////////////////////////////////


    unordered_set<shared_ptr<vector<int>>, PtrTupleHash, PtrTupleEq> new_tuples;

    auto it_1 = t1.tuples.begin();
    auto tuple = *it_1;

    auto it_2 = t2.tuples.begin();
    tuple = *it_2;

    if (columns1.size() == 0){
        /////////////////////////////////////////////////////////////
        // cout << "collect columns only from table 2" << endl;
        /////////////////////////////////////////////////////////////
        collect_indices(t2, columns2, new_tuples);
        for (auto idx : columns2){
            arg_idx.push_back(t2.tuple_index[idx]);
        }
    } else if(columns2.size() == 0){
        /////////////////////////////////////////////////////////////
        // cout << "collect columns from table 1 only" << endl;
        /////////////////////////////////////////////////////////////

        collect_indices(t1, columns1, new_tuples);
        for (auto idx : columns1){
            arg_idx.push_back(t1.tuple_index[idx]);
        }
    } else {
        if (matches1.empty()){
            /////////////////////////////////////////////////////////////
            // cout << "collect from both tables with no matching columns" << endl;
            /////////////////////////////////////////////////////////////

            for (shared_ptr<vector<int>> tuple1 : t1.tuples){
                for (shared_ptr<vector<int>> tuple2 : t2.tuples){
                    vector<int> new_tuple;
                    for (auto idx : columns1){
                        new_tuple.push_back(tuple1->at(idx));
                    }
                    for (auto idx : columns2){
                        new_tuple.push_back(tuple2->at(idx));
                    }
                    new_tuples.insert(make_shared<vector<int>>(new_tuple));
                }
            }
            for (auto idx : columns1){
                arg_idx.push_back(t1.tuple_index[idx]);
            }
            for (auto idx : columns2){
                arg_idx.push_back(t2.tuple_index[idx]);
            }
            
        }else{
            /////////////////////////////////////////////////////////////
            // cout << "collect from both tables with matching columns" << endl;
            /////////////////////////////////////////////////////////////

            unordered_map<vector<int>, vector<shared_ptr<vector<int>>>, TupleHash> hash_join_map;

            for (const shared_ptr<vector<int>> &tuple : t1.tuples){
                hash_join_map[ptr_project_tuple(*tuple, matches1)].push_back(tuple);
            }

            vector<bool> to_remove(t2.tuple_index.size(), false);
            for (const auto &m : matches2){
                to_remove[m] = true;
            }

            for(shared_ptr<vector<int>> tuple : t2.tuples){
                auto it = hash_join_map.find(ptr_project_tuple(*tuple, matches2));
                if (it != hash_join_map.end()) {
                    const auto matching_tuples = it->second;
                    for (shared_ptr<vector<int>> t : matching_tuples){
                        vector<int> new_tuple;
                        for (auto idx : columns1){
                            new_tuple.push_back(t->at(idx));
                        }
                        for ( auto idx : columns2){
                            new_tuple.push_back(tuple->at(idx));
                        }
                        new_tuples.insert(make_shared<vector<int>>(new_tuple));
                    }
                }
            }
            for (auto idx : columns1){
                arg_idx.push_back(t1.tuple_index[idx]);
            }
            for (auto idx : columns2){
                arg_idx.push_back(t2.tuple_index[idx]);
            }
        }
    }
    
    ////////////////////////////////////////////////////////////
    // cout << "new tuples" << endl;
    // cout << "___________________________" << endl;
    // for (auto tuple : new_tuples){
    //     for ( auto elem : *tuple){
    //         cout << "- " << elem << " - ";
    //     }
    //     cout << endl;
    // }
    // cout << "___________________________" << endl;
    ////////////////////////////////////////////////////////////

    t1.tuple_index = move(arg_idx);
    t1.tuples = move(new_tuples);
}
