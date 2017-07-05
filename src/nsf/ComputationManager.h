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

#ifdef DEPQBF_ENABLED
extern "C" 
{
#include "qdpll.h"
}
#endif

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
    RESULT decide(Computation& c);
    BDD solutions(Computation& c);

    void optimize(Computation &c);
    
    void incrementInternalAbstractCount();
    void incrementAbstractCount();
    void incrementShiftCount();
    
    void incrementSplitCount();

protected:

    void printStatistics() const;

private:
    Application& app;

    void divideGlobalNSFSizeEstimation(int value);
    void multiplyGlobalNSFSizeEstimation(int value);
    
    void updateStats(const Computation& c);

    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::DefaultIntegerValueOption optMaxGlobalNSFSize;
    options::DefaultIntegerValueOption optMaxBDDSize;
    options::DefaultIntegerValueOption optOptimizeInterval;
    options::DefaultIntegerValueOption optUnsatCheckInterval;
    options::Option optSortBeforeJoining;
    options::Choice optDependencyScheme;
    options::Option optDisableCache;

    double maxGlobalNSFSizeEstimation;

    unsigned int optIntervalCounter;
    bool left = true;

    unsigned int optUnsatCheckCounter;
    
    // for standard depencency scheme handling (provided by DepQBF)
#ifdef DEPQBF_ENABLED
    void initializeDepqbf();
    void initializeCuddToOriginalIds();
    std::vector<std::set < htd::vertex_t>> initializeNotYetRemovedAtLevels();
    QDPLL* depqbf = NULL;
    std::vector<unsigned int>* cuddToOriginalIds;
#endif
    
    // for simple dependency scheme handling
    void initializeVariableCountAtLevels();
    std::vector<unsigned int>* variableCountAtLevels;
    
    
    // Statistics
    unsigned int shiftCount;
    unsigned int internalAbstractCount;
    unsigned int abstractCount;
    
    unsigned int splitCount;
    
    unsigned int maxNSFsize;
    unsigned int maxNSFsizeBDDsize;
    unsigned int maxNSFsizeDomainSize;
    unsigned int maxNSFsizeCacheSize;
    
    unsigned int maxBDDsize;
    unsigned int maxBDDsizeNSFsize;
    unsigned int maxBDDsizeDomainSize;
    unsigned int maxBDDsizeCacheSize;
    
    unsigned int maxCacheSize;
    unsigned int maxCacheSizeNSFsize;
    unsigned int maxCacheSizeBDDsize;
    unsigned int maxCacheSizeDomainSize;
    
    unsigned int maxDomainSize;
    unsigned int maxDomainSizeNSFsize;
    unsigned int maxDomainSizeBDDsize;
    unsigned int maxDomainSizeCacheSize;
    
};




