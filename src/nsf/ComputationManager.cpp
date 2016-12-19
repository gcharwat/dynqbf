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

/**
 TODO:
 * elegant handling of when to optimize
 * elegant handling of maxBDDzie and maxNSFsize (refactor)
 * XXX: removeCache handling
 */

#include "../Application.h"
#include "ComputationManager.h"
#include "Computation.h"
#include "CacheComputation.h"

const std::string ComputationManager::NSFMANAGER_SECTION = "NSF Manager";

ComputationManager::ComputationManager(Application& app)
: app(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxGlobalNSFSize("max-est-NSF-size", "s", "Split until the global estimated NSF size <s> is reached, -1 to disable limit", 1000)
, optMaxBDDSize("max-BDD-size", "s", "Split if a BDD size exceeds <s> (may be overruled by max-est-NSF-size)", 3000)
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step,0 to disable", 4)
, optUnsatCheckInterval("unsat-check", "i", "Check for unsatisfiability after every <i>-th NSF join, 0 to disable", 0)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, maxGlobalNSFSizeEstimation(1)
, optIntervalCounter(0)
, optUnsatCheckCounter(0) {
    app.getOptionHandler().addOption(optOptimizeInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optUnsatCheckInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxGlobalNSFSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxBDDSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optSortBeforeJoining, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
}

ComputationManager::~ComputationManager() {
    printStatistics();
}

Computation* ComputationManager::newComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd) {
    // TODO: dynamically return Computation with or without cache
    bool keepFirstLevel = false;
    if (quantifierSequence.size() >= 1 && quantifierSequence.at(0) == NTYPE::EXISTS) {
        keepFirstLevel = app.enumerate();
    }
    return new CacheComputation(quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel);
    //return new Computation(quantifierSequence, cubesAtLevels, bdd);
}

Computation* ComputationManager::copyComputation(const Computation& c) {
    Computation* nC = new Computation(c);
    return nC;
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f) {
    c.apply(cubesAtLevels, f);
    optimize(c);
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    c.apply(cubesAtLevels, clauses);
    optimize(c);
}

void ComputationManager::conjunct(Computation& c, Computation& other) {
    divideGlobalNSFSizeEstimation(c.leavesCount());
    divideGlobalNSFSizeEstimation(other.leavesCount());
    if (optSortBeforeJoining.isUsed()) {
        c.sortByIncreasingSize();
        other.sortByIncreasingSize();
    }
    c.conjunct(other);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
    
    if (optUnsatCheckInterval.getValue() > 0) {
        optUnsatCheckCounter++;
        optUnsatCheckCounter %= optUnsatCheckInterval.getValue();

        if (optUnsatCheckCounter == 0) {
            RESULT result = decide(c);
            if (result == RESULT::UNSAT) {
                throw AbortException("Intermediate unsat check successful", RESULT::UNSAT);
            }
        }
    }
}

void ComputationManager::remove(Computation& c, const BDD& variable, const unsigned int vl) {
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(variable, vl);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices) {
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(removedVertices);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    //    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
    //        for (BDD variable : removedVertices.at(level - 1)) {
    //            divideGlobalNSFSizeEstimation(c.leavesCount());
    //            c.removeApply(variable, level, cubesAtLevels, clauses);
    //            multiplyGlobalNSFSizeEstimation(c.leavesCount());
    //            optimize(c);
    //        }
    //    }

    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.removeApply(removedVertices, cubesAtLevels, clauses);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::optimize(Computation &c) {
    if (optOptimizeInterval.getValue() > 0) {
        optIntervalCounter++;
        optIntervalCounter %= optOptimizeInterval.getValue();

        if (optIntervalCounter == 0) {
            while ((maxGlobalNSFSizeEstimation < optMaxGlobalNSFSize.getValue()) || (optMaxGlobalNSFSize.getValue() <= -1)) {
                divideGlobalNSFSizeEstimation(c.leavesCount());
                if (!(c.optimize(left))) {
                    break;
                }
                left = !left;
                multiplyGlobalNSFSizeEstimation(c.leavesCount());
            }
        }
    }
}

bool ComputationManager::isUnsat(const Computation& c) const {
    return c.isUnsat();
}

RESULT ComputationManager::decide(const Computation& c) const {
    return c.decide();
}

BDD ComputationManager::solutions(const Computation& c) const {
    return c.solutions();
}

void ComputationManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
    //std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    //std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    //std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}

void ComputationManager::divideGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation /= value;
    if (maxGlobalNSFSizeEstimation < 1) {
        maxGlobalNSFSizeEstimation = 1;
    }
}

void ComputationManager::multiplyGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation *= value;
}