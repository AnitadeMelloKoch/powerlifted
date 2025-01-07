#ifndef SEARCH_TABLE_H
#define SEARCH_TABLE_H

#include <vector>
#include <memory>
#include <unordered_set>

#include "../hash_structures.h"

/**
 * @brief Data-structure containing a set of tuples and the indices corresponding to
 * the free variable index of each tuple position.
 */

class Table {
public:
    using tuple_t = std::vector<int>;

    /// @var tuples: the relation corresponding to the table, encoded as a vector of tuples
    std::vector<tuple_t> tuples;
    /// @var tuple_index: Indices of each variable in order
    std::vector<int> tuple_index;
    // create table from dbstate
    Table(std::vector<tuple_t> &&tuples, std::vector<int> &&tuple_index) :
        tuples(std::move(tuples)),
        tuple_index(std::move(tuple_index))
    {}

    bool index_is_variable(std::size_t i) const {
        return tuple_index[i] >= 0;
    }

    Table() = default;

    static const Table& EMPTY_TABLE();
};

class PtrTable {
    public:
        using ptr_tuple_t = std::shared_ptr<std::vector<int>>;

        std::unordered_set<ptr_tuple_t, PtrTupleHash, PtrTupleEq> tuples;
        std::vector<int> tuple_index;

        bool index_is_variable(std::size_t i) const {
            return tuple_index[i] >= 0;
        }

        PtrTable(std::unordered_set<ptr_tuple_t, PtrTupleHash, PtrTupleEq> tuples, std::vector<int> &&tuple_index):
            tuples(tuples),
            tuple_index(std::move(tuple_index))
        {}

        PtrTable() = default;

        static const PtrTable& EMPTY_TABLE();
};

#endif //SEARCH_TABLE_H
