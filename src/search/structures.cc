#include "structures.h"
#include "hash_structures.h"
#include <vector>
#include <memory>
#include <unordered_set>

Relation::Relation(const PtrRelation &relation){
    tuples = std::unordered_set<GroundAtom, TupleHash>();
    for (auto tuple : relation.tuples){
        auto new_tuple = std::vector<int>();
        for (auto elem_ptr : *tuple){
            new_tuple.push_back(elem_ptr);
        } 
        tuples.insert(new_tuple);
    }

    predicate_symbol = relation.predicate_symbol;
}

PtrRelation::PtrRelation(const Relation &relation){
    tuples = std::unordered_set<std::shared_ptr<GroundAtom>, PtrTupleHash>();
    for (auto tuple : relation.tuples){
        auto new_tuple = std::vector<int>();
        for (auto &elem : tuple){
            new_tuple.push_back(elem);
        }
        tuples.insert(std::make_shared<GroundAtom>(new_tuple));
    }

    predicate_symbol = relation.predicate_symbol;
}

