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

#include "DependencyCacheComputation.h"
#include "cuddInt.h"

DependencyCacheComputation::DependencyCacheComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, QDPLL& depqbf, std::vector<unsigned int>& cuddToOriginalIds)
: CacheComputation(quantifierSequence, cubesAtLevels, bdd, maxBDDsize, keepFirstLevel)
, depqbf(depqbf)
, cuddToOriginalIds(cuddToOriginalIds) {
}

DependencyCacheComputation::DependencyCacheComputation(const DependencyCacheComputation& other)
: CacheComputation(other)
, depqbf(other.depqbf)
, cuddToOriginalIds(other.cuddToOriginalIds) {
}

DependencyCacheComputation::~DependencyCacheComputation() {
}

bool DependencyCacheComputation::reduceRemoveCache() {
    if (isRemoveCacheReducible()) {
        // TODO add options for removal strategies (orderings) here
        for (unsigned int vl = _removeCache->size(); vl >= 1; vl--) {
            if (isRemovableAtRemoveCacheLevel(vl)) {
                BDD toRemove = popFirstFromRemoveCache(vl); // simulate fifo
                unsigned int removedOriginalId = cuddToOriginalIds.at(toRemove.getRegularNode()->index);

                const std::vector<BDD>* variableDomain = getVariableDomain();

                bool dependent = false;
                for (unsigned int level = vl + 1; level <= variableDomain->size(); level++) {
                    BDD cubesAtLevel = variableDomain->at(level - 1);
                    if (isDependent(cubesAtLevel.getNode(), removedOriginalId)) {
                        dependent = true;
                        break;
                    }
                }

                if (dependent) {
                    Computation::remove(toRemove, vl);
                } else {
//                    std::cout << removedOriginalId << " has NO dependencies" << std::endl;

                    Computation::removeAbstract(toRemove, vl);
                }

                return true;

            }
        }
    }
    return false;
}

//void DependencyCacheComputation::reduceRemoveCache() {
//
//    const std::vector<BDD>* variableDomain = getVariableDomain();
//
//    //    CacheComputation::addToRemoveCache(variable, vl);
//    //    return;
//    //    
//
//    // always immediately remove innermost variables
//    if (vl == variableDomain->size()) {
//        Computation::remove(variable, vl);
//        return;
//    } else {
//        unsigned int removedOriginalId = cuddToOriginalIds.at(variable.getRegularNode()->index);
//
//
//        bool dependent = false;
//        for (unsigned int level = vl + 1; level <= variableDomain->size(); level++) {
//            BDD cubesAtLevel = variableDomain->at(level - 1);
//            if (isDependent(cubesAtLevel.getNode(), removedOriginalId)) {
//                dependent = true;
//                break;
//            }
//        }
//
//        if (dependent) {
//            //            std::cout << removedOriginalId << " has dependencies" << std::endl;
//            CacheComputation::addToRemoveCache(variable, vl);
//        } else {
//            std::cout << removedOriginalId << " has NO dependencies" << std::endl;
//
//            Computation::removeFromVariableDomain(variable, vl);
//            const std::vector<BDD> cubesAtLevels;
//            // TODO Quantors
//            Computation::apply(cubesAtLevels, [variable] (BDD bdd) -> BDD {
//                return bdd.UnivAbstract(variable);
//            });
//
//
//        }
//
//    }
//
//    //    variable.getRegularNode()->index;
//    //    if (qdpll_var_depends (depqbf, v1, v2)) {
//    //        
//    //    }
//
//    //    std::string vString = hypergraph->vertexName(v.getVertices()[0]);
//
//
//
//
//    //    // always immediately remove innermost variables
//    //    if (vl == _removeCache->size()) {
//    //        Computation::remove(variable, vl);
//    //        return;
//    //    }
//    //    if (_removeCache->size() < vl) {
//    //        for (unsigned int i = _removeCache->size(); i < vl; i++) {
//    //            std::vector<BDD> bddsAtLevel;
//    //            _removeCache->push_back(bddsAtLevel);
//    //        }
//    //    }
//    //    _removeCache->at(vl - 1).push_back(variable);
//}

bool DependencyCacheComputation::isDependent(DdNode* f, unsigned int removedOriginalVertexId) const {
    DdNode* g = Cudd_Regular(f);
    if (cuddIsConstant(g)) {
        return false;
    }

    unsigned int currentOriginalVertexId = cuddToOriginalIds.at(g->index);

//    std::cout << removedOriginalVertexId << " -?> " << currentOriginalVertexId;

    if (qdpll_var_depends(&depqbf, removedOriginalVertexId, currentOriginalVertexId)) {
//        std::cout << " yes" << std::endl;
        return true;
    }
//    std::cout << " no" << std::endl;

    DdNode* n = cuddT(g);
    if (!cuddIsConstant(n)) {
        if (isDependent(n, removedOriginalVertexId)) {
            return true;
        }
    }

    n = cuddE(g);
    DdNode* N = Cudd_Regular(n);
    if (!cuddIsConstant(N)) {
        if (isDependent(n, removedOriginalVertexId)) {
            return true;
        }
    }


    return false;
}