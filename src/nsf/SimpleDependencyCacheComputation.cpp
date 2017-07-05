/*
Copyright 2016-2017, Guenther Charwat
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

#include "SimpleDependencyCacheComputation.h"
#include "cuddInt.h"

SimpleDependencyCacheComputation::SimpleDependencyCacheComputation(ComputationManager& manager, const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, const std::vector<unsigned int>& variableCountAtLevels)
: CacheComputation(manager, quantifierSequence, cubesAtLevels, bdd, maxBDDsize, keepFirstLevel)
, _completelyRemovedCountAtLevel(quantifierSequence.size(), 0)
, _variableCountAtLevels(variableCountAtLevels) {
}

SimpleDependencyCacheComputation::SimpleDependencyCacheComputation(const SimpleDependencyCacheComputation& other)
: CacheComputation(other)
, _completelyRemovedCountAtLevel(other._completelyRemovedCountAtLevel)
, _variableCountAtLevels(other._variableCountAtLevels) {
}

SimpleDependencyCacheComputation::~SimpleDependencyCacheComputation() {
}

void SimpleDependencyCacheComputation::conjunct(const Computation& other) {
    CacheComputation::conjunct(other);
    try {
        // check if other contains a remove cache
        const SimpleDependencyCacheComputation& t = dynamic_cast<const SimpleDependencyCacheComputation&> (other);
        for (unsigned int i = 0; i < t._completelyRemovedCountAtLevel.size(); i++) {
            _completelyRemovedCountAtLevel[i] += t._completelyRemovedCountAtLevel.at(i);
        }
    } catch (std::bad_cast exp) {
    }
}

bool SimpleDependencyCacheComputation::reduceRemoveCache() {
    if (isRemoveCacheReducible()) {
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isRemovableAtRemoveCacheLevel(vl)) {
                BDD toRemove = popFirstFromRemoveCache(vl); // simulate fifo

                bool abstractable = isAbstractableAtLevel(vl);
                if (abstractable) {
                    Computation::removeAbstract(toRemove, vl);
                } else {
                    Computation::remove(toRemove, vl);
                }
                _completelyRemovedCountAtLevel.at(vl - 1) += 1;
                return true;
            }
        }
    }
    return false;
}

void SimpleDependencyCacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
    if (isAbstractableAtLevel(vl)) {
        Computation::removeAbstract(variable, vl);
        _completelyRemovedCountAtLevel.at(vl - 1) += 1;
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

bool SimpleDependencyCacheComputation::isAbstractableAtLevel(unsigned int vl) {
    bool isAbstractable = true;

    for (unsigned int i = vl; i < _variableCountAtLevels.size(); i++) {
        if (_completelyRemovedCountAtLevel.at(i) < _variableCountAtLevels.at(i)) {
            isAbstractable = false;
        }
    }

    return isAbstractable;
}