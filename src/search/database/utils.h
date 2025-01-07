
#ifndef DATABASE_HASH_JOIN_H
#define DATABASE_HASH_JOIN_H

#include <vector>
#include <utility>

class Table;
class PtrTable;

std::vector<std::pair<int, int>> compute_matching_columns(const Table &t1, const Table &t2);

void compute_matching_columns(const Table &t1, const Table &t2, std::vector<int>& matches1, std::vector<int>& matches2);
void ptr_compute_matching_columns(const PtrTable &t1, const PtrTable &t2, std::vector<int>& matches1, std::vector<int>& matches2);
void ptr_compute_relevant_columns(const PtrTable &t1, const PtrTable &t2, std::vector<int>& columns1, std::vector<int>& columns2, const std::vector<int> &relevant_args);

#endif //SEARCH_SEMI_JOIN_H
