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
#include <vector>
#include <algorithm>
#include <iterator>

#include "StandardDependencyCacheComputation.h"
#include "cuddInt.h"

StandardDependencyCacheComputation::StandardDependencyCacheComputation(ComputationManager& manager, const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, QDPLL& depqbf, std::vector<unsigned int>& cuddToOriginalIds, std::vector<std::set<htd::vertex_t>>&notYetRemovedAtLevels)
: CacheComputation(manager, quantifierSequence, cubesAtLevels, bdd, maxBDDsize, keepFirstLevel)
, _depqbf(depqbf)
, _cuddToOriginalIds(cuddToOriginalIds)
, _notYetRemovedAtLevels(notYetRemovedAtLevels) {
}

StandardDependencyCacheComputation::StandardDependencyCacheComputation(const StandardDependencyCacheComputation& other)
: CacheComputation(other)
, _depqbf(other._depqbf)
, _cuddToOriginalIds(other._cuddToOriginalIds) {
    for (std::set<htd::vertex_t> otherNotYetRemoved : other._notYetRemovedAtLevels) {
        std::set<htd::vertex_t> thisNotYestRemoved(otherNotYetRemoved);
        _notYetRemovedAtLevels.push_back(thisNotYestRemoved);
    }
}

StandardDependencyCacheComputation::~StandardDependencyCacheComputation() {
}

void StandardDependencyCacheComputation::conjunct(const Computation& other) {
    CacheComputation::conjunct(other);
    try {
        const StandardDependencyCacheComputation& t = dynamic_cast<const StandardDependencyCacheComputation&> (other);
        for (unsigned int i = 0; i < t._notYetRemovedAtLevels.size(); i++) {

            std::set<htd::vertex_t> own = _notYetRemovedAtLevels.at(i);
            std::set<htd::vertex_t> other = t._notYetRemovedAtLevels.at(i);
            std::set<htd::vertex_t> target;

            std::set_intersection(own.begin(), own.end(),
                    other.begin(), other.end(),
                    std::inserter(target, target.begin()));

            _notYetRemovedAtLevels.at(i) = target;
        }
    } catch (std::bad_cast exp) {
    }
}

bool StandardDependencyCacheComputation::reduceRemoveCache() {
    if (isRemoveCacheReducible()) {
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isRemovableAtRemoveCacheLevel(vl)) {
                BDD toRemove = popFirstFromRemoveCache(vl); // simulate fifo
                unsigned int removedOriginalId = _cuddToOriginalIds.at(toRemove.getRegularNode()->index);

                bool dependent = false;
                unsigned int independentUntilLevel = vl;

                for (unsigned int level = vl + 1; level <= _notYetRemovedAtLevels.size(); level++) {
                    for (htd::vertex_t notYetRemoved : _notYetRemovedAtLevels.at(level - 1)) {
                        if (qdpll_var_depends(&_depqbf, removedOriginalId, notYetRemoved)) {
                            dependent = true;
                            break;
                        }
                    }
                    if (dependent) {
                        break;
                    } else {
                        independentUntilLevel = level;
                    }
                }

                if (dependent) {
                    if (independentUntilLevel >= (vl + 2)) {
                        if ((independentUntilLevel - vl) % 2 == 1) {
                            independentUntilLevel--;
                        }
                        shiftVariableLevel(toRemove, vl, independentUntilLevel);
                        manager.incrementShiftCount();
                    }
                    Computation::remove(toRemove, independentUntilLevel); // instead of vl
                } else {
                    // we abstract at vl since it does not make a difference
                    Computation::removeAbstract(toRemove, vl); // instead of vl
                    // only remove if it is abstracted
                    _notYetRemovedAtLevels.at(vl - 1).erase(removedOriginalId); // here we remove at vl, since variable was not shifted in notYetRemovedAtLevels
                }
                return true;
            }
        }
    }
    return false;
}

void StandardDependencyCacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
    unsigned int removedOriginalId = _cuddToOriginalIds.at(variable.getRegularNode()->index);

    bool dependent = false;
    for (unsigned int level = vl + 1; level <= _notYetRemovedAtLevels.size(); level++) {
        for (htd::vertex_t notYetRemoved : _notYetRemovedAtLevels.at(level - 1)) {
            if (qdpll_var_depends(&_depqbf, removedOriginalId, notYetRemoved)) {
                dependent = true;
                break;
            }
        }
        if (dependent) {
            break;
        }
    }
    
    // TODO: only if we do not enumerate or level > 1!
    if (!dependent && !(_keepFirstLevel && vl == 1)) {
        Computation::removeAbstract(variable, vl);
        _notYetRemovedAtLevels.at(vl - 1).erase(removedOriginalId);
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

void StandardDependencyCacheComputation::print(bool verbose) const {
    std::cout << "Not yet removed at levels (size):" << std::endl;
    for (unsigned int level = 1; level <= _notYetRemovedAtLevels.size(); level++) {
        std::cout << level << ": " << _notYetRemovedAtLevels.at(level - 1).size() << std::endl;
    }
    CacheComputation::print(verbose);
}
