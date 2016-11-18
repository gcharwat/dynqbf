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
#include "TmpComputationManager.h"
//#include "Computation.h"
#include "TmpCacheComputation.h"

const std::string TmpComputationManager::NSFMANAGER_SECTION = "NSF Manager";

TmpComputationManager::TmpComputationManager(Application& app)
: app(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxNSFSize("max-NSF-size", "s", "Split until NSF size <s> is reached", 1000)
, optMaxBDDSize("max-BDD-size", "s", "Always split if a BDD size exceeds <s> (overrules max-NSF-size)", 40000)
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step", 3)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, subsetChecks(0)
, subsetChecksSuccessful(0)
, maxNSFSizeEstimation(1)
, optIntervalCounter(0) {
    app.getOptionHandler().addOption(optOptimizeInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxNSFSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxBDDSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optSortBeforeJoining, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
}

TmpComputationManager::~TmpComputationManager() {
    printStatistics();
}

TmpComputation* TmpComputationManager::newComputation(const std::vector<NTYPE>& quantifierSequence, const BDD& bdd) {
    // TODO: dynamically return Computation with or without cache
    return new TmpCacheComputation(quantifierSequence, bdd, optMaxNSFSize.getValue(), optMaxBDDSize.getValue());
    //return new TmpComputation(quantifierSequence, bdd);
}

TmpComputation* TmpComputationManager::copyComputation(const TmpComputation& c) {
    TmpComputation* nC = new TmpComputation(c);
//    multiplyMaxNSFSizeEstimation(nC->leavesCount());
    return nC;
}

void TmpComputationManager::apply(TmpComputation& c, std::function<BDD(const BDD&)> f) {
    c.apply(f);
}

void TmpComputationManager::apply(TmpComputation& c, const BDD& clauses) {
    c.apply(clauses);
}

void TmpComputationManager::conjunct(TmpComputation& c, const TmpComputation& other) {
    c.conjunct(other);
////    if (c1.maxBDDsize() > optMaxBDDSize.getValue()) {
////        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
////        int oldSize = c1.leavesCount();
////        maxNSFSizeEstimation = 0;
////        //split(c1);
////        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
////        if (maxNSFSizeEstimation <= 0) {
////            maxNSFSizeEstimation = 1;
////        }
////        maxNSFSizeEstimation *= c1.leavesCount();
////    }
////    if (c2.maxBDDsize() > optMaxBDDSize.getValue()) {
////        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
////        int oldSize = c2.leavesCount();
////        maxNSFSizeEstimation = 0;
////        //split(c2);
////        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
////        if (maxNSFSizeEstimation <= 0) {
////            maxNSFSizeEstimation = 1;
////        }
////        maxNSFSizeEstimation *= c2.leavesCount();
////    }
//
//    divideMaxNSFSizeEstimation(c1.leavesCount());
//    divideMaxNSFSizeEstimation(c2.leavesCount());
//    Computation* nC = BaseNSFManager::conjunct(c1, c2);
//    // TODO: propagate SORTBEFOREJOINING
//    multiplyMaxNSFSizeEstimation(nC->leavesCount());
//    
//    reduceRemoveCache(*nC);
//    return nC;
}

void TmpComputationManager::remove(TmpComputation& c, const BDD& variable, const unsigned int vl) {
    c.remove(variable, vl);
}

void TmpComputationManager::remove(TmpComputation& c, const std::vector<std::vector<BDD>>& removedVertices) {
    c.remove(removedVertices);
}

void TmpComputationManager::removeApply(TmpComputation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) {
    c.removeApply(removedVertices, clauses);
}

void TmpComputationManager::optimize(TmpComputation &c) {
    optIntervalCounter++;
    optIntervalCounter %= optOptimizeInterval.getValue();
    
    if (optIntervalCounter == 0) {
        c.optimize();
    }
////    rotateCheck++;
////    if (optimizeNow(false) || optimizeNow(true)) {
//        divideMaxNSFSizeEstimation(c.leavesCount());
//        BaseNSFManager::optimize(c);
//        multiplyMaxNSFSizeEstimation(c.leavesCount());
////    }
}

//bool HeuristicNSFManager::split(Computation& c) {
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
//
//}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
//int TmpComputationManager::compressConjunctive(Computation &c) {
//    int localSubsetChecksSuccessful = BaseNSFManager::compressConjunctive(c);
//    subsetChecksSuccessful += localSubsetChecksSuccessful;
//    return localSubsetChecksSuccessful;
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

const BDD TmpComputationManager::evaluate(const TmpComputation& c, std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) {
    return c.evaluate(app, cubesAtlevels, keepFirstLevel);
}

void TmpComputationManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
    std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}

void TmpComputationManager::divideMaxNSFSizeEstimation(int value) {
    maxNSFSizeEstimation /= value;
    if (maxNSFSizeEstimation < 1) {
        maxNSFSizeEstimation = 1;
    }
}
    
void TmpComputationManager::multiplyMaxNSFSizeEstimation(int value) {
    maxNSFSizeEstimation *= value;
}