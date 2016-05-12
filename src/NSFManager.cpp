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
#include "NSFManager.h"
#include "Computation.h"

const std::string NSFManager::NSFMANAGER_SECTION = "NSF Manager";

NSFManager::NSFManager(Application& app)
: app(app)
, quantifierSequence()
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxNSFSize("max-NSF-size", "s", "Split until NSF size <s> is reached (Recommended: 1000)")
, optMaxBDDSize("max-BDD-size", "s", "Always split if a BDD size exceeds <s> (Recommended: 3000, overrules max-NSF-size)")
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step (default: 4, disable: 0)")
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

NSFManager::~NSFManager() {
    printStatistics();
}

void NSFManager::init(std::vector<NTYPE> quantifierSequence) {
    this->quantifierSequence = quantifierSequence;
}

Computation* NSFManager::newComputation(unsigned int level, BDD bdd) const {
    unsigned int depth = quantifierCount() - level;
    Computation* c = new Computation(level, depth, quantifier(level));
    if (depth == 0) {
        c->setValue(bdd);
    } else {
        Computation* cC = newComputation(level + 1, bdd);
        c->insert(cC);
    }
    if (level == 1) {
        maxNSFSizeEstimation *= c->leavesCount();
    }
    return c;
}

Computation* NSFManager::copyComputation(const Computation& c) const {
    Computation* nC = new Computation(c.level(), c.depth(), c.quantifier());
    std::copy(c.removeCache().begin(), c.removeCache().end(), std::inserter(nC->mutableRemoveCache(), nC->mutableRemoveCache().begin()));

    if (c.isLeaf()) {
        nC->setValue(c.value());
    } else {
        for (const Computation* cC : c.nestedSet()) {
            Computation* ncC = copyComputation(*cC);
            nC->insert(ncC);
        }
    }
    if (nC->level() == 1) {
        maxNSFSizeEstimation *= nC->leavesCount();
    }
    return nC;
}

void NSFManager::apply(Computation& c, std::function<BDD(const BDD&)> f) const {
    if (c.isLeaf()) {
        c._value = std::move(f(c.value()));
    } else {
        for (Computation* cC : c.nestedSet()) {
            apply(*cC, f);
        }
    }
}

Computation* NSFManager::conjunct(Computation& c1, Computation& c2) const {
    if (optMaxBDDSize.isUsed()) {
        if (c1.maxBDDsize() > (std::stoi(optMaxBDDSize.getValue()))) {
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
        if (c2.maxBDDsize() > (std::stoi(optMaxBDDSize.getValue()))) {
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
    }


    Computation* nC = new Computation(c1.level(), c1.depth(), c1.quantifier());
    std::copy(c1.removeCache().begin(), c1.removeCache().end(), std::inserter(nC->mutableRemoveCache(), nC->mutableRemoveCache().begin()));
    for (BDD b : c2.removeCache()) {
        if (std::find(nC->removeCache().begin(), nC->removeCache().end(), b) == nC->removeCache().end()) {
            nC->addToRemoveCache(b);
        }
    }

    if (c1.isLeaf()) {
        nC->_value = std::move(c1.value() * c2.value());
    } else {

        // sort by size
        std::list<Computation*> cl1(c1.nestedSet().begin(), c1.nestedSet().end());
        std::list<Computation*> cl2(c2.nestedSet().begin(), c2.nestedSet().end());
        if (optSortBeforeJoining.isUsed() && (optimizeNow(true) || optimizeNow(false))) {
            cl1.sort([] (const Computation* c1, const Computation * c2) -> bool {
                long unsigned int c1Size = (c1->isLeaf() ? c1->maxBDDsize() : c1->nestedSet().size());
                long unsigned int c2Size = (c2->isLeaf() ? c2->maxBDDsize() : c2->nestedSet().size());
                return (c1Size < c2Size);
            });
            cl2.sort([] (const Computation* c1, const Computation * c2) -> bool {
                long unsigned int c1Size = (c1->isLeaf() ? c1->maxBDDsize() : c1->nestedSet().size());
                long unsigned int c2Size = (c2->isLeaf() ? c2->maxBDDsize() : c2->nestedSet().size());
                return (c1Size < c2Size);
            });
        }

        for (Computation* cC1 : cl1) {
            std::list<Computation*> newSet;
            std::list<Computation *>::iterator itn;

            for (Computation* cC2 : cl2) {
                Computation* ncC = conjunct(*cC1, *cC2);
                bool deleteIt1 = false;
                bool deleteIt2 = false;

                for (itn = newSet.begin(); itn != newSet.end();) {
                    Computation* ocC = (*itn);

                    if (nC->depth() > 1 || nC->isUniversiallyQuantified()) {
                        if (optimizeNow(true)) {
                            if (*(ocC) <= *(ncC)) {
                                subsetChecksSuccessful++;
                                deleteIt2 = true;
                            }
                        } else if (optimizeNow(false)) {
                            if (*(ncC) <= *(ocC)) {
                                subsetChecksSuccessful++;
                                deleteIt1 = true;
                            }
                        }
                    } else {
                        if (optimizeNow(false)) {
                            if (*(ncC) <= *(ocC)) {
                                subsetChecksSuccessful++;
                                deleteIt2 = true;
                            }
                        } else if (optimizeNow(true)) {
                            if (*(ocC) <= *(ncC)) {
                                subsetChecksSuccessful++;
                                deleteIt1 = true;
                            }
                        }
                    }

                    if (deleteIt2) {
                        break;
                    } else if (deleteIt1) {
                        delete *itn;
                        itn = newSet.erase(itn);
                        deleteIt1 = false;
                    } else {
                        itn++;
                    }
                }
                if (deleteIt2) {
                    delete ncC;
                } else {
                    newSet.push_back(ncC);
                }

            }
            for (Computation* newC : newSet) {
                nC->insert(newC);
            }
        }
    }
    if (nC->level() == 1) {
        maxNSFSizeEstimation /= c1.leavesCount();
        maxNSFSizeEstimation /= c2.leavesCount();
        if (maxNSFSizeEstimation <= 0) {
            maxNSFSizeEstimation = 1;
        }
        maxNSFSizeEstimation *= nC->leavesCount();
    }
    return nC;
}

void NSFManager::removeApply(Computation& c, std::vector<std::vector<BDD>> removedVertices, const BDD& clauses) const {
    maxNSFSizeEstimation /= c.leavesCount();
    if (maxNSFSizeEstimation <= 0) {
        maxNSFSizeEstimation = 1;
    }
    removeApplyRec(c, removedVertices, app.getBDDManager().getManager().bddOne(), clauses);
    maxNSFSizeEstimation *= c.leavesCount();
}

void NSFManager::removeApplyRec(Computation& c, std::vector<std::vector<BDD>> removedVertices, BDD restrict, const BDD& clauses) const {
    if (c.isLeaf()) {
        BDD newBdd = c.value().Restrict(restrict);
        newBdd = newBdd * clauses;
        BDD cube = app.getBDDManager().getManager().bddOne();
        for (BDD b : removedVertices[c.level() - 1]) {
            cube *= b;
        }
        if (c.isExistentiallyQuantified()) {
            c._value = newBdd.ExistAbstract(cube, 0);
        } else {
            c._value = newBdd.UnivAbstract(cube);
        }
    } else {
        if (removedVertices[c.level() - 1].empty()) {
            for (Computation* cC : c.nestedSet()) {
                removeApplyRec(*cC, removedVertices, restrict, clauses);
            }
        } else {
            BDD removed = removedVertices[c.level() - 1].back();
            removedVertices[c.level() - 1].pop_back();

            BDD newClauses1 = clauses.Restrict(removed);
            BDD newClauses2 = clauses.Restrict(!removed);

            Computation* cop;
            cop = copyComputation(c);
            removeApplyRec(*cop, removedVertices, (restrict*!removed), newClauses2);
            removeApplyRec(c, removedVertices, (restrict * removed), newClauses1);
            for (Computation* newC : cop->nestedSet()) {
                c.insert(newC);
            }
            cop->_nestedSet.clear();
            delete cop;
        }
    }
}

void NSFManager::remove(Computation& c, const BDD& variable, const unsigned int vl) const {
    int oldSize = c.leavesCount();
    maxNSFSizeEstimation *= oldSize;
    removeRec(c, variable, vl);
    maxNSFSizeEstimation /= oldSize;
    maxNSFSizeEstimation /= oldSize;
    if (maxNSFSizeEstimation <= 0) {
        maxNSFSizeEstimation = 1;
    }
    maxNSFSizeEstimation *= c.leavesCount();
}

void NSFManager::removeRec(Computation& c, const BDD& variable, const unsigned int vl) const {
    if (c.level() == vl) {
        c.addToRemoveCache(variable);
        bool splitting = split(c); // XXX should not be here
        if (splitting) optimize(c);
    } else {
        for (Computation* cC : c.nestedSet()) {
            removeRec(*cC, variable, vl);
        }
    }
}

void NSFManager::optimize(Computation &c) const {
    rotateCheck++;
    if (optimizeNow(false) || optimizeNow(true)) {
        maxNSFSizeEstimation /= c.leavesCount();
        optimizeRec(c);
        maxNSFSizeEstimation *= c.leavesCount();
    }
}

void NSFManager::optimizeRec(Computation &c) const {
    if (!c.isLeaf()) {
        for (Computation* cC : c.nestedSet()) {
            optimizeRec(*cC);
        }
    }
    compressConjunctive(c);
}

bool NSFManager::split(Computation& c) const {
    if (c.removeCache().empty()) {
        return false;
    }
    if (!c.isLeaf() && optMaxNSFSize.isUsed() && (maxNSFSizeEstimation > (std::stoi(optMaxNSFSize.getValue())))) {
        return false;
    }
    if (app.enumerate() && c.level() == 1 && c.quantifier() == NTYPE::EXISTS) {
        return false;
    }
    if (c.isLeaf()) {
        BDD cube = app.getBDDManager().getManager().bddOne();
        for (BDD b : c.removeCache()) cube *= b;
        c._removeCache.clear();
        if (c.isExistentiallyQuantified()) {
            c._value = std::move(c.value().ExistAbstract(cube, 0));
        } else {
            c._value = std::move(c.value().UnivAbstract(cube));
        }
    } else {
        BDD variable = c.removeCache().back();
        c._removeCache.pop_back();

        std::vector<Computation*> newComps;
        for (Computation* cC : c.nestedSet()) {
            Computation* cop = copyComputation(*cC);
            apply(*cC, [&variable] (BDD b) -> BDD {
                return b.Restrict(variable);
            });
            apply(*cop, [&variable] (BDD b) -> BDD {
                return b.Restrict(!variable);
            });
            newComps.push_back(cop);
        }
        for (Computation* newC : newComps) {
            c.insert(newC);
        }
    }
    return true;
}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
void NSFManager::compressConjunctive(Computation &c) const {
    int depth = c.depth();

    if (depth == 0) {
        return;
    } else {
        std::list<Computation *> list(c.nestedSet().begin(), c.nestedSet().end());
        
        std::list<Computation*>::iterator it1;
        std::list<Computation*>::iterator it2;

        // TODO Size
        // TODO Removal cache

        std::list<Computation *>::const_iterator end = list.end();
        for (it1 = list.begin(); it1 != end;) {
            Computation* c1 = *it1;
            bool deleteIt1 = false;
            it2 = it1;
            it2++;
            while (it2 != end) {
                Computation& c2 = *(*it2);
                // TODO special handling for innermost quantifier
                // to be fixed when q-resolution is implemented
                bool deleteIt2 = false;
                if (depth > 1 || c.isUniversiallyQuantified()) {
                    if (optimizeNow(false)) {
                        if (*(c1) <= c2) {
                            subsetChecksSuccessful++;
                            deleteIt2 = true;
                        }
                    } else if (optimizeNow(true)) {
                        if (c2 <= *(c1)) {
                            subsetChecksSuccessful++;
                            deleteIt1 = true;
                        }
                    }

                } else {
                    if (optimizeNow(true)) {
                        if (c2 <= *(c1)) {
                            subsetChecksSuccessful++;
                            deleteIt2 = true;
                        }
                    } else if (optimizeNow(false)) {
                        if (*(c1) <= c2) {
                            subsetChecksSuccessful++;
                            deleteIt1 = true;
                        }
                    }

                }
                if (deleteIt2) {
                    delete *it2;
                    it2 = list.erase(it2);
                    end = list.end();
                } else if (deleteIt1) {
                    delete *it1;
                    it1 = list.erase(it1);
                    c1 = *it1;
                    end = list.end();
                    break;
                } else {
                    it2++;
                }
            }
            if (!deleteIt1) {
                it1++;
            }
        }
        c._nestedSet.clear();
        std::vector<Computation *> tmp(std::make_move_iterator(list.begin()), std::make_move_iterator(list.end()));
        c._nestedSet = tmp;

    }
}

const BDD NSFManager::evaluateNSF(const std::vector<BDD>& cubesAtlevels, const Computation& c, bool keepFirstLevel) const {
    BDD ret;
    if (c.isLeaf()) {
        ret = c.value();
    } else {
        if (c.isExistentiallyQuantified()) {
            ret = app.getBDDManager().getManager().bddZero();
            for (Computation * cC : c.nestedSet()) {
                ret += evaluateNSF(cubesAtlevels, *cC, keepFirstLevel);
            }
        } else {
            ret = app.getBDDManager().getManager().bddOne();
            for (Computation * cC : c.nestedSet()) {
                ret *= evaluateNSF(cubesAtlevels, *cC, keepFirstLevel);
            }
        }
    }

    int levelIndex = c.level() - 1;
    if (levelIndex != 0 || !keepFirstLevel) {
        if (c.isExistentiallyQuantified()) {
            ret = ret.ExistAbstract(cubesAtlevels[levelIndex], 0);
            for (BDD b : c.removeCache()) {
                ret = ret.ExistAbstract(b, 0);
            }
        } else {
            ret = ret.UnivAbstract(cubesAtlevels[levelIndex]);
            for (BDD b : c.removeCache()) {

                ret = ret.UnivAbstract(b);
            }
        }
    }

    return ret;
}

void NSFManager::pushBackQuantifier(const NTYPE quantifier) {
    quantifierSequence.push_back(quantifier);
}

void NSFManager::pushFrontQuantifier(const NTYPE quantifier) {
    quantifierSequence.insert(quantifierSequence.begin(), quantifier);
}

const NTYPE NSFManager::innermostQuantifier() const {
    return quantifier(quantifierCount());
}

const NTYPE NSFManager::quantifier(const unsigned int level) const {
    if (level < 1 || level > quantifierCount()) {
        return NTYPE::UNKNOWN;
    }
    return quantifierSequence[level - 1];
}

const unsigned int NSFManager::quantifierCount() const {
    return quantifierSequence.size();
}

void NSFManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
    std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}

bool NSFManager::optimizeNow(bool half) const {
    int rotateCheckInterval = 4;
    if (optOptimizeInterval.isUsed()) {
        rotateCheckInterval = ((std::stoi) (optOptimizeInterval.getValue()));
    }
    if (rotateCheckInterval <= 0) {
        return false;
    }
    bool checking = false;
    rotateCheck = rotateCheck % rotateCheckInterval;
    if (!half) {
        checking = (rotateCheck % rotateCheckInterval == rotateCheckInterval);
    } else {
        checking = (rotateCheck % rotateCheckInterval == rotateCheckInterval / 2);
    }
    if (checking) {
        subsetChecks++;
    }
    return checking;
}