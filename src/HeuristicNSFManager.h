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
    
    virtual Computation* copyComputation(const Computation& c) const override;
    virtual Computation* conjunct(Computation& c1, Computation& c2) const override;
    virtual void remove(Computation& c, const BDD& variable, const unsigned int vl) const override;

    
    virtual void removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) const override;


    void optimize(Computation &c) const;


//    void printStatistics() const;

protected:

    bool split(Computation& c) const;
    int compressConjunctive(Computation &c) const;

private:
    void printStatistics() const;

//    void removeApplyRec(Computation& c, std::vector<std::vector<BDD>> removedVertices, BDD restrict, const BDD& clauses) const;
//    void optimizeRec(Computation &c) const;
    void removeRec(Computation& c, const BDD& variable, const unsigned int vl) const;
//    bool optimizeNow(bool half) const;

//    mutable std::vector<Computation*> computationStore;
    
    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::DefaultIntegerValueOption optMaxNSFSize;
    options::DefaultIntegerValueOption optMaxBDDSize;
    options::DefaultIntegerValueOption optOptimizeInterval;
    options::Option optSortBeforeJoining;
    
    mutable unsigned long subsetChecks;
    mutable unsigned long subsetChecksSuccessful;
    mutable int maxNSFSizeEstimation;

};




