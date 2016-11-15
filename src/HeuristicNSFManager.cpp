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

#include "Application.h"
#include "HeuristicNSFManager.h"
#include "Computation.h"

const std::string HeuristicNSFManager::NSFMANAGER_SECTION = "NSF Manager";

HeuristicNSFManager::HeuristicNSFManager(Application& app)
: BaseNSFManager(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxNSFSize("max-NSF-size", "s", "Split until NSF size <s> is reached", 1000)
, optMaxBDDSize("max-BDD-size", "s", "Always split if a BDD size exceeds <s> (overrules max-NSF-size)", 3000)
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step", 4)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, subsetChecks(0)
, subsetChecksSuccessful(0)
, maxNSFSizeEstimation(1) {
    app.getOptionHandler().addOption(optOptimizeInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxNSFSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxBDDSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optSortBeforeJoining, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
}

HeuristicNSFManager::~HeuristicNSFManager() {
    printStatistics();
}

Computation* HeuristicNSFManager::copyComputation(const Computation& c) const {
    Computation* nC = BaseNSFManager::copyComputation(c);
    maxNSFSizeEstimation *= nC->leavesCount();
    return nC;
}

Computation* HeuristicNSFManager::conjunct(Computation& c1, Computation& c2) const {
    if (c1.maxBDDsize() > optMaxBDDSize.getValue()) {
        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
        int oldSize = c1.leavesCount();
        maxNSFSizeEstimation = 0;
        split(c1);
        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
        if (maxNSFSizeEstimation <= 0) {
            maxNSFSizeEstimation = 1;
        }
        maxNSFSizeEstimation *= c1.leavesCount();
    }
    if (c2.maxBDDsize() > optMaxBDDSize.getValue()) {
        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
        int oldSize = c2.leavesCount();
        maxNSFSizeEstimation = 0;
        split(c2);
        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
        if (maxNSFSizeEstimation <= 0) {
            maxNSFSizeEstimation = 1;
        }
        maxNSFSizeEstimation *= c2.leavesCount();
    }

    Computation* nC = BaseNSFManager::conjunct(c1, c2);
    // TODO: propagate SORTBEFOREJOINING

    maxNSFSizeEstimation /= c1.leavesCount();
    maxNSFSizeEstimation /= c2.leavesCount();
    if (maxNSFSizeEstimation <= 0) {
        maxNSFSizeEstimation = 1;
    }
    maxNSFSizeEstimation *= nC->leavesCount();
    return nC;
}

void HeuristicNSFManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) const {
    maxNSFSizeEstimation /= c.leavesCount();
    if (maxNSFSizeEstimation <= 0) {
        maxNSFSizeEstimation = 1;
    }
    BaseNSFManager::removeApply(c, removedVertices, clauses);
    maxNSFSizeEstimation *= c.leavesCount();
}

void HeuristicNSFManager::remove(Computation& c, const BDD& variable, const unsigned int vl) const {
    int oldSize = c.leavesCount();
    maxNSFSizeEstimation *= oldSize;
    BaseNSFManager::remove(c, variable, vl);
    maxNSFSizeEstimation /= oldSize;
    maxNSFSizeEstimation /= oldSize;
    if (maxNSFSizeEstimation <= 0) {
        maxNSFSizeEstimation = 1;
    }
    maxNSFSizeEstimation *= c.leavesCount();    
}

void HeuristicNSFManager::optimize(Computation &c) const {
//    rotateCheck++;
//    if (optimizeNow(false) || optimizeNow(true)) {
        maxNSFSizeEstimation /= c.leavesCount();
        BaseNSFManager::optimize(c);
        maxNSFSizeEstimation *= c.leavesCount();
//    }
}

bool HeuristicNSFManager::split(Computation& c) const {
    return false; // TODo
//    if (c.removeCache().empty()) {
//        return false;
//    }
//    if (!c.isLeaf() && (maxNSFSizeEstimation > optMaxNSFSize.getValue())) {
//        return false;
//    }
//    if (app.enumerate() && c.level() == 1 && c.quantifier() == NTYPE::EXISTS) {
//        return false;
//    }
//    if (c.isLeaf()) {
//        BDD cube = app.getBDDManager().getManager().bddOne();
//        for (BDD b : c.removeCache()) cube *= b;
//        c._removeCache.clear();
//        if (c.isExistentiallyQuantified()) {
//            c._value = std::move(c.value().ExistAbstract(cube, 0));
//        } else {
//            c._value = std::move(c.value().UnivAbstract(cube));
//        }
//        return false;
//    } else {
//        BDD variable = c.removeCache().back();
//        c._removeCache.pop_back();
//
//        std::vector<Computation*> newComps;
//        for (Computation* cC : c.nestedSet()) {
//            Computation* cop = copyComputation(*cC);
//            apply(*cC, [&variable] (BDD b) -> BDD {
//                return b.Restrict(variable);
//            });
//            apply(*cop, [&variable] (BDD b) -> BDD {
//                return b.Restrict(!variable);
//            });
//            newComps.push_back(cop);
//        }
//        for (Computation* newC : newComps) {
//            c.insert(newC);
//        }
//        return true;
//    }

}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
int HeuristicNSFManager::compressConjunctive(Computation &c) const {
    int localSubsetChecksSuccessful= BaseNSFManager::compressConjunctive(c);
    subsetChecksSuccessful += localSubsetChecksSuccessful;
    return localSubsetChecksSuccessful;
}


//void NSFManager::pushBackQuantifier(const NTYPE quantifier) {
//    quantifierSequence.push_back(quantifier);
//}
//
//void NSFManager::pushFrontQuantifier(const NTYPE quantifier) {
//    quantifierSequence.insert(quantifierSequence.begin(), quantifier);
//}
//
//const NTYPE NSFManager::innermostQuantifier() const {
//    return quantifier(quantifierCount());
//}
//
//const NTYPE NSFManager::quantifier(const unsigned int level) const {
//    if (level < 1 || level > quantifierCount()) {
//        return NTYPE::UNKNOWN;
//    }
//    return quantifierSequence[level - 1];
//}
//
//const unsigned int NSFManager::quantifierCount() const {
//    return quantifierSequence.size();
//}



//bool NSFManager::optimizeNow(bool half) const {
//    int rotateCheckInterval = optOptimizeInterval.getValue();
//    if (rotateCheckInterval <= 0) {
//        return false;
//    }
//    bool checking = false;
//    rotateCheck = rotateCheck % rotateCheckInterval;
//    if (!half) {
//        checking = (rotateCheck % rotateCheckInterval == rotateCheckInterval);
//    } else {
//        checking = (rotateCheck % rotateCheckInterval == rotateCheckInterval / 2);
//    }
//    if (checking) {
//        subsetChecks++;
//    }
//    return checking;
//}

void HeuristicNSFManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
    std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}