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

DependencyCacheComputation::DependencyCacheComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, QDPLL& depqbf, std::vector<Variable>& variables)
: CacheComputation(quantifierSequence, cubesAtLevels, bdd, maxBDDsize, keepFirstLevel)
, depqbf(depqbf)
, variables(variables) {
}

DependencyCacheComputation::DependencyCacheComputation(const DependencyCacheComputation& other)
: CacheComputation(other)
, depqbf(other.depqbf)
, variables(other.variables) {
}

DependencyCacheComputation::~DependencyCacheComputation() {
}


void DependencyCacheComputation::addToRemoveCache(BDD variable, const unsigned int vl) {
//    variable.getRegularNode()->index;
//    if (qdpll_var_depends (depqbf, v1, v2)) {
//        
//    }
    
//    std::string vString = hypergraph->vertexName(v.getVertices()[0]);
    
    CacheComputation::addToRemoveCache(variable, vl);
    
    
    
//    // always immediately remove innermost variables
//    if (vl == _removeCache->size()) {
//        Computation::remove(variable, vl);
//        return;
//    }
//    if (_removeCache->size() < vl) {
//        for (unsigned int i = _removeCache->size(); i < vl; i++) {
//            std::vector<BDD> bddsAtLevel;
//            _removeCache->push_back(bddsAtLevel);
//        }
//    }
//    _removeCache->at(vl - 1).push_back(variable);
}
