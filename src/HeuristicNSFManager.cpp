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
    
    removeCache = new std::vector<std::vector<BDD>>;
}

HeuristicNSFManager::~HeuristicNSFManager() {
    delete removeCache;
    printStatistics();
}

Computation* HeuristicNSFManager::newComputation(const BDD& bdd) {
    return BaseNSFManager::newComputation(bdd);
}

Computation* HeuristicNSFManager::copyComputation(const Computation& c) {
    Computation* nC = BaseNSFManager::copyComputation(c);
    multiplyMaxNSFSizeEstimation(nC->leavesCount());
    return nC;
}

void HeuristicNSFManager::apply(Computation& c, std::function<BDD(const BDD&)> f) {
    BaseNSFManager::apply(c,f);
}

void HeuristicNSFManager::apply(Computation& c, const BDD& clauses) {
    BaseNSFManager::apply(c,clauses);
}

Computation* HeuristicNSFManager::conjunct(Computation& c1, Computation& c2) {
//    if (c1.maxBDDsize() > optMaxBDDSize.getValue()) {
//        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
//        int oldSize = c1.leavesCount();
//        maxNSFSizeEstimation = 0;
//        //split(c1);
//        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
//        if (maxNSFSizeEstimation <= 0) {
//            maxNSFSizeEstimation = 1;
//        }
//        maxNSFSizeEstimation *= c1.leavesCount();
//    }
//    if (c2.maxBDDsize() > optMaxBDDSize.getValue()) {
//        int maxNSFSizeEstimationTmp = maxNSFSizeEstimation;
//        int oldSize = c2.leavesCount();
//        maxNSFSizeEstimation = 0;
//        //split(c2);
//        maxNSFSizeEstimation = maxNSFSizeEstimationTmp / oldSize;
//        if (maxNSFSizeEstimation <= 0) {
//            maxNSFSizeEstimation = 1;
//        }
//        maxNSFSizeEstimation *= c2.leavesCount();
//    }

    divideMaxNSFSizeEstimation(c1.leavesCount());
    divideMaxNSFSizeEstimation(c2.leavesCount());
    Computation* nC = BaseNSFManager::conjunct(c1, c2);
    // TODO: propagate SORTBEFOREJOINING
    multiplyMaxNSFSizeEstimation(nC->leavesCount());
    
    reduceRemoveCache(*nC);
    return nC;
}

void HeuristicNSFManager::remove(Computation& c, const BDD& variable, const unsigned int vl) {
    addToRemoveCache(variable, vl);
    reduceRemoveCache(c);
}

void HeuristicNSFManager::remove(Computation& c, const std::vector<std::vector<BDD>>& removedVertices) {
    addToRemoveCache(removedVertices);
    reduceRemoveCache(c);
}

void HeuristicNSFManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) {
    addToRemoveCache(removedVertices);
    reduceRemoveCache(c);
    apply(c, clauses);
}

void HeuristicNSFManager::optimize(Computation &c) {
//    rotateCheck++;
//    if (optimizeNow(false) || optimizeNow(true)) {
        divideMaxNSFSizeEstimation(c.leavesCount());
        BaseNSFManager::optimize(c);
        multiplyMaxNSFSizeEstimation(c.leavesCount());
//    }
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
int HeuristicNSFManager::compressConjunctive(Computation &c) {
    int localSubsetChecksSuccessful = BaseNSFManager::compressConjunctive(c);
    subsetChecksSuccessful += localSubsetChecksSuccessful;
    return localSubsetChecksSuccessful;
}

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

const BDD HeuristicNSFManager::evaluateNSF(const Computation& c, const std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) {
    return BaseNSFManager::evaluateNSF(c, cubesAtlevels, keepFirstLevel);
}

void HeuristicNSFManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
    std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}

bool HeuristicNSFManager::isRemoveCacheReducible(Computation& c) {
    if (isEmptyRemoveCache()) {
        return false;
    } else if (c.maxBDDsize() > optMaxBDDSize.getValue()) {
        return true;
    } else if (maxNSFSizeEstimation < optMaxNSFSize.getValue()) {
        return true;
    }
    return false;
}

void HeuristicNSFManager::reduceRemoveCache(Computation& c) {
    while(isRemoveCacheReducible(c)) {
        // TODO add options for removal strategies (orderings) here
        for (unsigned int vl = removeCache->size(); vl >= 1; vl--) {
            if (isEmptyAtRemoveCacheLevel(vl)) {
                continue;
            }
            BDD toRemove = popFromRemoveCache(vl);
            divideMaxNSFSizeEstimation(c.leavesCount());
            BaseNSFManager::remove(c, toRemove, vl);    
            multiplyMaxNSFSizeEstimation(c.leavesCount());
            break;
        }
    }
}

void HeuristicNSFManager::addToRemoveCache(BDD variable, const unsigned int vl) {
    if (removeCache->size() < vl) {
        for (unsigned int i = removeCache->size(); i < vl; i++) {
            std::vector<BDD> bddsAtLevel;
            removeCache->push_back(bddsAtLevel);
        }
    }
    removeCache->at(vl - 1).push_back(variable);
}

void HeuristicNSFManager::addToRemoveCache(const std::vector<std::vector<BDD>>& variables) {
    for (unsigned int vl = 1; vl <= variables.size(); vl++) {
        for (BDD variable : variables[vl-1]) {
            addToRemoveCache(variable, vl);
        }
    }
}

BDD HeuristicNSFManager::popFromRemoveCache(const unsigned int vl) {
    if (isEmptyAtRemoveCacheLevel(vl)) {
        return app.getBDDManager().getManager().bddOne();
    }
    BDD b = removeCache->at(vl - 1).back();
    removeCache->at(vl - 1).pop_back();
    return b;
}

bool HeuristicNSFManager::isEmptyRemoveCache() {
    for (unsigned int vl = 1; vl <= removeCache->size(); vl++) {
        if (!isEmptyAtRemoveCacheLevel(vl))
            return false;
    }
    return true;
}

bool HeuristicNSFManager::isEmptyAtRemoveCacheLevel(const unsigned int vl) {
    if (removeCache->size() < vl) {
        return true;
    }
    return removeCache->at(vl - 1).empty();
}

void HeuristicNSFManager::divideMaxNSFSizeEstimation(int value) {
    maxNSFSizeEstimation /= value;
    if (maxNSFSizeEstimation < 1) {
        maxNSFSizeEstimation = 1;
    }
}
    
void HeuristicNSFManager::multiplyMaxNSFSizeEstimation(int value) {
    maxNSFSizeEstimation *= value;
}