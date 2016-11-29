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

#include "CacheComputation.h"

CacheComputation::CacheComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize)
: Computation(quantifierSequence, cubesAtLevels, bdd)
, _maxBDDsize(maxBDDsize) {
    _removeCache = new std::vector<std::vector < BDD >> (quantifierSequence.size());
}

CacheComputation::CacheComputation(const CacheComputation& other)
: Computation(other)
, _maxBDDsize(other._maxBDDsize) {
    _removeCache = new std::vector<std::vector < BDD >> (other._removeCache->size());
    for (unsigned int level = 1; level <= other._removeCache->size(); level++) {
        std::copy(_removeCache->at(level - 1).begin(), other._removeCache->at(level - 1).begin(), other._removeCache->at(level - 1).end());
    }
}

CacheComputation::~CacheComputation() {
    if (_removeCache != NULL)
        delete _removeCache;
}

void CacheComputation::conjunct(const Computation& other) {
    Computation::conjunct(other);
    try {
        // check if other contains a remove cache
        const CacheComputation& t = dynamic_cast<const CacheComputation&> (other);
        addToRemoveCache(*(t._removeCache));
    } catch (std::bad_cast exp) {
    }
}

void CacheComputation::remove(const BDD& variable, const unsigned int vl) {
    addToRemoveCache(variable, vl);
}

void CacheComputation::remove(const std::vector<std::vector<BDD>>&removedVertices) {
    addToRemoveCache(removedVertices);
}

void CacheComputation::removeApply(const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    addToRemoveCache(removedVertices);
    apply(cubesAtLevels, clauses);
}

BDD CacheComputation::evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const {
    for (unsigned int level = 1; level <= _removeCache->size(); level++) {
        for (BDD b : _removeCache->at(level - 1)) {
            if (cubesAtlevels.size() < level) {
                cubesAtlevels.push_back(b);
            } else {
                cubesAtlevels.at(level - 1) *= b;
            }
        }
    }
    return Computation::evaluate(cubesAtlevels, keepFirstLevel);
}

RESULT CacheComputation::decide() const {
    std::vector<BDD> cubesAtlevels;
    BDD decide = evaluate(cubesAtlevels, false);
    if (decide.IsZero()) {
        return RESULT::UNSAT;
    } else if (decide.IsOne()) {
        return RESULT::SAT;
    } else {
        // decide.print(0, 2);
        return RESULT::UNDECIDED;
    }
}

BDD CacheComputation::solutions() const {
    std::vector<BDD> cubesAtlevels;
    return evaluate(cubesAtlevels, false);
}

bool CacheComputation::optimize() {
    if (reduceRemoveCache()) {
        Computation::optimize();
        return true;
    }
    return false;
}

bool CacheComputation::optimize(bool left) {
    if (reduceRemoveCache()) {
        Computation::optimize(left);
        return true;
    }
    return false;
}

void CacheComputation::print() const {
    std::cout << "Remove cache (size):" << std::endl;
    for (unsigned int level = 1; level <= _removeCache->size(); level++) {
        std::cout << level << ": " << _removeCache->at(level - 1).size() << std::endl;
    }
    Computation::print();
}

bool CacheComputation::isRemoveCacheReducible() {
    if (isEmptyRemoveCache()) {
        return false;
    } else if (_nsf->maxBDDsize() > _maxBDDsize) {
        return true;
    }
    return false;
}

bool CacheComputation::reduceRemoveCache() {
    if (isRemoveCacheReducible()) {
        // TODO add options for removal strategies (orderings) here
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isEmptyAtRemoveCacheLevel(vl)) {
                continue;
            }
            BDD toRemove = popFirstFromRemoveCache(vl); // simulate fifo
            Computation::remove(toRemove, vl);
            return true;
        }
    }
    return false;
}

void CacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
    // always immediately remove innermost variables
    if (vl == _removeCache->size()) {
        Computation::remove(variable, vl);
        return;
    }
    if (_removeCache->size() < vl) {
        for (unsigned int i = _removeCache->size(); i < vl; i++) {
            std::vector<BDD> bddsAtLevel;
            _removeCache->push_back(bddsAtLevel);
        }
    }
    _removeCache->at(vl - 1).push_back(variable);
}

void CacheComputation::addToRemoveCache(const std::vector<std::vector<BDD>>&variables) {
    for (unsigned int vl = 1; vl <= variables.size(); vl++) {
        for (BDD variable : variables.at(vl - 1)) {
            addToRemoveCache(variable, vl);
        }
    }
}

BDD CacheComputation::popFromRemoveCache(const unsigned int vl) {
    //    if (isEmptyAtRemoveCacheLevel(vl)) {
    //        return app.getBDDManager().getManager().bddOne();
    //    }
    BDD b = _removeCache->at(vl - 1).back();
    _removeCache->at(vl - 1).pop_back();
    return b;
}

BDD CacheComputation::popFirstFromRemoveCache(const unsigned int vl) {
    //    if (isEmptyAtRemoveCacheLevel(vl)) {
    //        return app.getBDDManager().getManager().bddOne();
    //    }
    std::iter_swap(_removeCache->at(vl - 1).begin(), _removeCache->at(vl - 1).end() - 1);
    BDD b = _removeCache->at(vl - 1).back();
    _removeCache->at(vl - 1).pop_back();
    return b;
}

bool CacheComputation::isEmptyRemoveCache() {
    for (unsigned int vl = 1; vl <= _removeCache->size(); vl++) {
        if (!isEmptyAtRemoveCacheLevel(vl))
            return false;
    }
    return true;
}

bool CacheComputation::isEmptyAtRemoveCacheLevel(const unsigned int vl) {
    if (_removeCache->size() < vl) {
        return true;
    }
    return _removeCache->at(vl - 1).empty();
}