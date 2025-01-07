
#include "hash_structures.h"

#include <boost/functional/hash/hash.hpp>

std::size_t TupleHash::operator()(const std::vector<int> &c) const {
    return boost::hash_range(c.begin(), c.end());
}

std::size_t PtrTupleHash::operator()(const std::shared_ptr<std::vector<int>> c) const {
    return boost::hash_range(c->begin(), c->end());
}

bool PtrTupleEq::operator()(const std::shared_ptr<std::vector<int>> a, const std::shared_ptr<std::vector<int>> b) const{
    if (a->size() != b->size()){
        return false;
    }

    for (size_t i = 0; i < a->size(); i++){
        if (a->at(i) != b->at(i)){
            return false;
        }
    }

    return true;
}
