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
, depqbf(depqbf)
, cuddToOriginalIds(cuddToOriginalIds)
, notYetRemovedAtLevels(notYetRemovedAtLevels) {
}

StandardDependencyCacheComputation::StandardDependencyCacheComputation(const StandardDependencyCacheComputation& other)
: CacheComputation(other)
, depqbf(other.depqbf)
, cuddToOriginalIds(other.cuddToOriginalIds) {
    for (std::set<htd::vertex_t> otherNotYetRemoved : other.notYetRemovedAtLevels) {
        std::set<htd::vertex_t> thisNotYestRemoved(otherNotYetRemoved);
        notYetRemovedAtLevels.push_back(thisNotYestRemoved);
    }
}

StandardDependencyCacheComputation::~StandardDependencyCacheComputation() {
}

void StandardDependencyCacheComputation::conjunct(const Computation& other) {
    CacheComputation::conjunct(other);
    try {
        const StandardDependencyCacheComputation& t = dynamic_cast<const StandardDependencyCacheComputation&> (other);
        // TODO: add checks not necessary
        for (unsigned int i = 0; i < t.notYetRemovedAtLevels.size(); i++) {

            std::set<htd::vertex_t> own = notYetRemovedAtLevels.at(i);
            std::set<htd::vertex_t> other = t.notYetRemovedAtLevels.at(i);
            std::set<htd::vertex_t> target;

            std::set_intersection(own.begin(), own.end(),
                    other.begin(), other.end(),
                    std::inserter(target, target.begin()));

            notYetRemovedAtLevels.at(i) = target;
        }
    } catch (std::bad_cast exp) {
    }
}

bool StandardDependencyCacheComputation::reduceRemoveCache() {
    if (isRemoveCacheReducible()) {
        // TODO add options for removal strategies (orderings) here
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isRemovableAtRemoveCacheLevel(vl)) {
                BDD toRemove = popFirstFromRemoveCache(vl); // simulate fifo
                unsigned int removedOriginalId = cuddToOriginalIds.at(toRemove.getRegularNode()->index);

                bool dependent = false;

                unsigned int independentUntilLevel = vl;

                for (unsigned int level = vl + 1; level <= notYetRemovedAtLevels.size(); level++) {

                    for (htd::vertex_t notYetRemoved : notYetRemovedAtLevels.at(level - 1)) {
                        if (qdpll_var_depends(&depqbf, removedOriginalId, notYetRemoved)) {
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
                    manager.incrementAbstractCount();
                    if (vl < notYetRemovedAtLevels.size()) {
                        manager.incrementInternalAbstractCount();
                    }

                    // we abstract at vl since it does not make a difference
                    Computation::removeAbstract(toRemove, vl); // instead of vl
                    // only remove if it is abstracted (hack)
                    notYetRemovedAtLevels.at(vl - 1).erase(removedOriginalId); // here we remove at vl, since variable was not shifted in notYetRemovedAtLevels
                }
                return true;
            }
        }
    }
    return false;
}

void StandardDependencyCacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
    unsigned int removedOriginalId = cuddToOriginalIds.at(variable.getRegularNode()->index);

    bool dependent = false;
    for (unsigned int level = vl + 1; level <= notYetRemovedAtLevels.size(); level++) {
        for (htd::vertex_t notYetRemoved : notYetRemovedAtLevels.at(level - 1)) {
            if (qdpll_var_depends(&depqbf, removedOriginalId, notYetRemoved)) {
                dependent = true;
                break;
            }
        }
        if (dependent) {
            break;
        }
    }
    if (!dependent) {
        Computation::removeAbstract(variable, vl);
        notYetRemovedAtLevels.at(vl - 1).erase(removedOriginalId);

        manager.incrementAbstractCount();
        if (vl < notYetRemovedAtLevels.size()) {
            manager.incrementInternalAbstractCount();
            //            std::cout << removedOriginalId << " immediately abstract at level " << vl << std::endl;
        }
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
    for (unsigned int level = 1; level <= notYetRemovedAtLevels.size(); level++) {
        std::cout << level << ": " << notYetRemovedAtLevels.at(level - 1).size() << std::endl;
    }
    CacheComputation::print(verbose);
}
