/*
Copyright 2016, Guenther Charwat
WWW: <http://dbai.tuwien.ac.at/proj/decodyn/dynqbf>.

This file is part of dynQBF.

dynQBF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

dynQBF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dynQBF.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <iostream>

#include "TmpCacheComputation.h"

TmpCacheComputation::TmpCacheComputation(const std::vector<NTYPE>& quantifierSequence, const BDD& bdd, unsigned int maxNSFsize, unsigned int maxBDDsize)
: TmpComputation(quantifierSequence, bdd)
, maxNSFsize(maxNSFsize)
, maxBDDsize(maxBDDsize) {
    _removeCache = new std::vector<std::vector<BDD>>(quantifierSequence.size());
}

TmpCacheComputation::TmpCacheComputation(const TmpCacheComputation& other) 
: TmpComputation(other) {
    _removeCache = new std::vector<std::vector<BDD>>(other._removeCache->size());
    for(unsigned int level = 1; level <= other._removeCache->size(); level++) {
        std::copy(_removeCache->at(level - 1).begin(), other._removeCache->at(level - 1).begin(), other._removeCache->at(level - 1).end());
    }
}

TmpCacheComputation::~TmpCacheComputation() {
    delete _removeCache;
}

void TmpCacheComputation::conjunct(const TmpComputation& other) {
    TmpComputation::conjunct(other);
    try {
        // check if other contains a remove cache
        const TmpCacheComputation& t = dynamic_cast<const TmpCacheComputation&>(other);
        addToRemoveCache(*(t._removeCache));
    } catch(std::bad_cast exp) {}
    
}

void TmpCacheComputation::remove(const BDD& variable, const unsigned int vl) {
    addToRemoveCache(variable, vl);
    reduceRemoveCache();
}

void TmpCacheComputation::remove(const std::vector<std::vector<BDD>>& removedVertices) {
    addToRemoveCache(removedVertices);
    reduceRemoveCache();
}

void TmpCacheComputation::removeApply(const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) {
    addToRemoveCache(removedVertices);
    reduceRemoveCache();
    apply(clauses);
}

const BDD TmpCacheComputation::evaluate(Application& app, std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const {
    for (unsigned int level = 1; level <= cubesAtlevels.size(); level++) {
        for (BDD b : _removeCache->at(level-1)) {
            cubesAtlevels.at(level-1) *= b;
        }
    }
    return _nsf->evaluate(app, cubesAtlevels, keepFirstLevel);
}

void TmpCacheComputation::optimize() {
    TmpComputation::optimize();
}

void TmpCacheComputation::print() const {
    std::cout << "Remove cache (size):" << std::endl;
    for (unsigned int level = 1; level <= _removeCache->size(); level++) {
        std::cout << level << ": " << _removeCache->at(level - 1).size() << std::endl;
    }
    TmpComputation::print();
}

bool TmpCacheComputation::isRemoveCacheReducible() {
    if (isEmptyRemoveCache()) {
        return false;
    } else if (_nsf->maxBDDsize() > maxBDDsize) {
        return true;
    } else if (_nsf->nsfCount() < maxNSFsize) {
        return true;
    }
    return false;
}

void TmpCacheComputation::reduceRemoveCache() {
    while(isRemoveCacheReducible()) {
        // TODO add options for removal strategies (orderings) here
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isEmptyAtRemoveCacheLevel(vl)) {
                continue;
            }
            BDD toRemove = popFromRemoveCache(vl);
            TmpComputation::remove(toRemove, vl);
            break;
        }
    }
}

void TmpCacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
    if (_removeCache->size() < vl) {
        for (unsigned int i = _removeCache->size(); i < vl; i++) {
            std::vector<BDD> bddsAtLevel;
            _removeCache->push_back(bddsAtLevel);
        }
    }
    _removeCache->at(vl - 1).push_back(variable);
}

void TmpCacheComputation::addToRemoveCache(const std::vector<std::vector<BDD>>& variables) {
    for (unsigned int vl = 1; vl <= variables.size(); vl++) {
        for (BDD variable : variables.at(vl-1)) {
            addToRemoveCache(variable, vl);
        }
    }
}

BDD TmpCacheComputation::popFromRemoveCache(const unsigned int vl) {
//    if (isEmptyAtRemoveCacheLevel(vl)) {
//        return app.getBDDManager().getManager().bddOne();
//    }
    BDD b = _removeCache->at(vl - 1).back();
    _removeCache->at(vl - 1).pop_back();
    return b;
}

bool TmpCacheComputation::isEmptyRemoveCache() {
    for (unsigned int vl = 1; vl <= _removeCache->size(); vl++) {
        if (!isEmptyAtRemoveCacheLevel(vl))
            return false;
    }
    return true;
}

bool TmpCacheComputation::isEmptyAtRemoveCacheLevel(const unsigned int vl) {
    if (_removeCache->size() < vl) {
        return true;
    }
    return _removeCache->at(vl - 1).empty();
}