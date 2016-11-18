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
#include "options/DefaultIntegerValueOption.h"
#include "cuddObj.hh"
#include "Instance.h"

class Application;
class TmpComputation;
class TmpCacheComputation;

class TmpComputationManager {
public:
    TmpComputationManager(Application& app);
    ~TmpComputationManager();
    
    TmpComputation* newComputation(const std::vector<NTYPE>& quantifierSequence, const BDD& bdd);
    TmpComputation* copyComputation(const TmpComputation& c);
    
    void apply(TmpComputation& c, std::function<BDD(const BDD&)> f);
    void apply(TmpComputation& c, const BDD& clauses);
    
    void conjunct(TmpComputation& c, const TmpComputation& other);
    
    void remove(TmpComputation& c, const BDD& variable, const unsigned int vl);
    void remove(TmpComputation& c, const std::vector<std::vector<BDD>>& removedVertices);
    void removeApply(TmpComputation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses);
    
    const BDD evaluate(const TmpComputation& c, std::vector<BDD>& cubesAtlevels, bool keepFirstLevel);

    void optimize(TmpComputation &c);

protected:

    void printStatistics() const;

private:
    Application& app;
    
    void divideMaxNSFSizeEstimation(int value);
    void multiplyMaxNSFSizeEstimation(int value);
    
    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::DefaultIntegerValueOption optMaxNSFSize;
    options::DefaultIntegerValueOption optMaxBDDSize;
    options::DefaultIntegerValueOption optOptimizeInterval;
    options::Option optSortBeforeJoining;
    
    unsigned long subsetChecks;
    unsigned long subsetChecksSuccessful;
    long maxNSFSizeEstimation;
    
    unsigned int optIntervalCounter;

};



