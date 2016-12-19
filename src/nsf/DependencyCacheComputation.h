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

#include "NSF.h"
#include "CacheComputation.h"
#include "../Variable.h"

extern "C" 
{
#include "qdpll.h"
}


#pragma once

class DependencyCacheComputation : public CacheComputation {
public:
    DependencyCacheComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, QDPLL& depqbf, std::vector<unsigned int>& cuddToOriginalIds);
    DependencyCacheComputation(const DependencyCacheComputation& other);

    ~DependencyCacheComputation();

protected:

    void addToRemoveCache(BDD variable, const unsigned int vl) override;

private:
    
    bool isDependent(DdNode* n, unsigned int originalVertexId) const;
    
    QDPLL& depqbf;
    std::vector<unsigned int>& cuddToOriginalIds;
};
