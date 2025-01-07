#ifndef SEARCH_HASH_STRUCTURES_H
#define SEARCH_HASH_STRUCTURES_H

#include <vector>
#include <memory>


/**
 * @brief Generic hash function for vector of integers. The name "TupleHash" is
 * used because its main function is to hash tuples in relations/tables.
 *
 * @note Uses boost hash_range function
 */
struct TupleHash {
  std::size_t operator()(const std::vector<int> &c) const;
};

struct PtrTupleHash {
    std::size_t operator()(const std::shared_ptr<std::vector<int>> c) const;
};

struct PtrTupleEq {
    bool operator()(const std::shared_ptr<std::vector<int>> a, const std::shared_ptr<std::vector<int>> b) const;
};

#endif // SEARCH_HASH_STRUCTURES_H
