
#include "utils.h"
#include "table.h"
#include "../hash_structures.h"

#include <iostream>

using namespace std;

std::vector<std::pair<int, int>> compute_matching_columns(const Table &t1, const Table &t2) {
    vector<pair<int, int>> matches;
    auto sz1 = t1.tuple_index.size();
    auto sz2 = t2.tuple_index.size();
    for (size_t i = 0; i < sz1; ++i) {
        for (size_t j = 0; j < sz2; ++j) {
            if (t1.tuple_index[i] == t2.tuple_index[j]) {
                matches.emplace_back(i, j);
            }
        }
    }
    return matches;
}

void compute_matching_columns(const Table &t1, const Table &t2, std::vector<int>& matches1, std::vector<int>& matches2) {
    vector<pair<int, int>> matches;
    auto sz1 = t1.tuple_index.size();
    auto sz2 = t2.tuple_index.size();
    for (size_t i = 0; i < sz1; ++i) {
        for (size_t j = 0; j < sz2; ++j) {
            if (t1.tuple_index[i] == t2.tuple_index[j]) {
                matches1.push_back(i);
                matches2.push_back(j);
            }
        }
    }
}

void ptr_compute_matching_columns(const PtrTable &t1, const PtrTable &t2, vector<int>& matches1, vector<int>& matches2){
    auto sz1 = t1.tuple_index.size();
    auto sz2 = t2.tuple_index.size();
    for (size_t i = 0; i < sz1; ++i){
        for (size_t j = 0; j < sz2; ++j){
            if (t1.tuple_index[i] == t2.tuple_index[j]){
                matches1.push_back(i);
                matches2.push_back(j);
            }
        }
    }
}

void ptr_compute_relevant_columns(const PtrTable &t1, const PtrTable &t2, vector<int>& columns1, vector<int>& columns2, const vector<int> &relevant_args){
    auto size1 = t1.tuple_index.size();
    auto size2 = t2.tuple_index.size();
    auto max_size = max(size1, size2);
    for (size_t i = 0; i < relevant_args.size(); i++){
        bool found = false;
        for (size_t j = 0; j < max_size; j++){
            if ((j < size1) && (!found)){
                if (relevant_args[i] == t1.tuple_index[j]){
                    columns1.push_back(j);
                    found = true;
                }
            }
            if ((j < size2) && (!found)){
                if (relevant_args[i] == t2.tuple_index[j]){
                    columns2.push_back(j);
                    found = true;
                }
            }
        }
    }
}
