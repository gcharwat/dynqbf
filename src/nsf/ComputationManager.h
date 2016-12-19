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

#pragma once

#include <map>

#include <cuddObj.hh>

#include "../Utils.h"
#include "../Application.h"
#include "../options/DefaultIntegerValueOption.h"
#include "../Instance.h"
#include "../AbortException.h"
#include "Computation.h"
#include "CacheComputation.h"
#include "../Variable.h"

extern "C" 
{
#include "qdpll.h"
}

class Application;

class ComputationManager {
public:
    ComputationManager(Application& app);
    ~ComputationManager();

    Computation* newComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd);
    Computation* copyComputation(const Computation& c);

    void apply(Computation& c, const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f);
    void apply(Computation& c, const std::vector<BDD>& cubesAtLevels, const BDD& clauses);

    void conjunct(Computation& c, Computation& other);

    void remove(Computation& c, const BDD& variable, const unsigned int vl);
    void remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices);
    void removeApply(Computation& c, const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses);

    bool isUnsat(const Computation& c) const;
    RESULT decide(const Computation& c) const;
    BDD solutions(const Computation& c) const;

    void optimize(Computation &c);

protected:

    void printStatistics() const;

private:
    Application& app;

    void divideGlobalNSFSizeEstimation(int value);
    void multiplyGlobalNSFSizeEstimation(int value);

    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::DefaultIntegerValueOption optMaxGlobalNSFSize;
    options::DefaultIntegerValueOption optMaxBDDSize;
    options::DefaultIntegerValueOption optOptimizeInterval;
    options::DefaultIntegerValueOption optUnsatCheckInterval;
    options::Option optSortBeforeJoining;
    options::Option optWithDependencyScheme;

    long maxGlobalNSFSizeEstimation;

    unsigned int optIntervalCounter;
    bool left = true;

    unsigned int optUnsatCheckCounter;
    
    QDPLL* depqbf = NULL;
    std::vector<unsigned int>* cuddToOriginalIds;
};




