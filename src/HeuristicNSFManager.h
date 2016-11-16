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

#include "Utils.h"
#include "Application.h"
#include "Computation.h"
#include "BaseNSFManager.h"
#include "options/DefaultIntegerValueOption.h"

class Application;
class Computation;

class HeuristicNSFManager : public BaseNSFManager {
public:
    HeuristicNSFManager(Application& app);
    ~HeuristicNSFManager();
    
    Computation* newComputation(const BDD& bdd) override;
    Computation* copyComputation(const Computation& c) override;
    
    void apply(Computation& c, std::function<BDD(const BDD&)> f) override;
    void apply(Computation& c, const BDD& clauses) override;
    
    Computation* conjunct(Computation& c1, Computation& c2) override;
    
    void remove(Computation& c, const BDD& variable, const unsigned int vl) override;
    void remove(Computation& c, const std::vector<std::vector<BDD>>& removedVertices) override;
    void removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) override;
    
    const BDD evaluateNSF(const Computation& c, const std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) override;

    void optimize(Computation &c) override;

protected:

    int compressConjunctive(Computation &c) override;
    void printStatistics() const;

private:
    
    void addToRemoveCache(BDD variable, const unsigned int vl);
    void addToRemoveCache(const std::vector<std::vector<BDD>>& variables);
    BDD popFromRemoveCache(const unsigned int vl);
    bool isEmptyAtRemoveCacheLevel(const unsigned int vl);
    bool isEmptyRemoveCache();
    
    void divideMaxNSFSizeEstimation(int value);
    void multiplyMaxNSFSizeEstimation(int value);
    
    bool isRemoveCacheReducible(Computation& c);
    void reduceRemoveCache(Computation& c);
    

    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::DefaultIntegerValueOption optMaxNSFSize;
    options::DefaultIntegerValueOption optMaxBDDSize;
    options::DefaultIntegerValueOption optOptimizeInterval;
    options::Option optSortBeforeJoining;
    
    unsigned long subsetChecks;
    unsigned long subsetChecksSuccessful;
    long maxNSFSizeEstimation;

    std::vector<std::vector<BDD>>* removeCache;
};




