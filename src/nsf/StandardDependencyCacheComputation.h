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
#pragma once

#include "NSF.h"
#include "CacheComputation.h"
#include "ComputationManager.h"

extern "C" 
{
#include "qdpll.h"
}

class StandardDependencyCacheComputation : public CacheComputation {
public:
    StandardDependencyCacheComputation(ComputationManager& manager, const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel, QDPLL& depqbf, std::vector<unsigned int>& cuddToOriginalIds, std::vector<std::set<htd::vertex_t>>& notYetRemovedAtLevels);
    StandardDependencyCacheComputation(const StandardDependencyCacheComputation& other);

    ~StandardDependencyCacheComputation();

    virtual void conjunct(const Computation& other) override;
    
    virtual void print(bool verbose) const override;
    
protected:
    bool reduceRemoveCache() override;
    void addToRemoveCache(BDD variable, const unsigned int vl) override;

private:
    
    QDPLL& _depqbf;
    std::vector<unsigned int>& _cuddToOriginalIds;
    
    std::vector<std::set<htd::vertex_t>> _notYetRemovedAtLevels;
    
};
