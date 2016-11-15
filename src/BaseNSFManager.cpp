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
#include "BaseNSFManager.h"
#include "Computation.h"

BaseNSFManager::BaseNSFManager(Application& app)
: app(app) {
}

BaseNSFManager::~BaseNSFManager() {
}

Computation* BaseNSFManager::newComputation(const BDD& bdd) const {
    return newComputationRec(1, bdd);
}

/* TODO remove recursion */
Computation* BaseNSFManager::newComputationRec(unsigned int level, const BDD& bdd) const {
    unsigned int depth = app.getInputInstance()->quantifierCount() - level;
    NTYPE quantifier = app.getInputInstance()->quantifier(level);
    Computation* c = new Computation(level, depth, quantifier);
    if (depth == 0) {
        c->setValue(bdd);
    } else {
        Computation* cC = newComputationRec(level + 1, bdd);
        c->insert(cC);
    }
    return c;
}

/* TODO mmove to Computation.cpp */
Computation* BaseNSFManager::copyComputation(const Computation& c) const {
    Computation* nC = new Computation(c.level(), c.depth(), c.quantifier());

    if (c.isLeaf()) {
        nC->setValue(c.value());
    } else {
        for (const Computation* cC : c.nestedSet()) {
            Computation* ncC = copyComputation(*cC);
            nC->insert(ncC);
        }
    }
    return nC;
}

void BaseNSFManager::apply(Computation& c, std::function<BDD(const BDD&)> f) const {
    if (c.isLeaf()) {
        c._value = std::move(f(c.value()));
    } else {
        for (Computation* cC : c.nestedSet()) {
            apply(*cC, f);
        }
    }
}

void BaseNSFManager::apply(Computation& c, const BDD& clauses) const {
    apply(c, [&clauses](BDD bdd) -> BDD {
        return bdd *= clauses;
    });
}

Computation* BaseNSFManager::conjunct(Computation& c1, Computation& c2) const {
    Computation* nC = new Computation(c1.level(), c1.depth(), c1.quantifier());

    if (c1.isLeaf()) {
        nC->_value = std::move(c1.value() * c2.value());
    } else {
        for (Computation* cC1 : c1.nestedSet()) {
            for (Computation* cC2 : c2.nestedSet()) {
                Computation* ncC = conjunct(*cC1, *cC2);
                nC->insert(ncC);
            }
        }
    }
    return nC;
}

void BaseNSFManager::remove(Computation& c, const BDD& variable, const unsigned int vl) const {
    if (c.level() == vl) {
        if (c.isLeaf()) {
            if (c.isExistentiallyQuantified()) {
                c._value = std::move(c.value().ExistAbstract(variable, 0));
            } else {
                c._value = std::move(c.value().UnivAbstract(variable));
            }
        } else {
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
    } else {
        for (Computation* cC : c.nestedSet()) {
            remove(*cC, variable, vl);
        }
    }
}

void BaseNSFManager::remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices) const {
    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
        for (BDD b : removedVertices[level - 1]) {
            remove(c, b, level);
        }
    }
}

void BaseNSFManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) const {
    apply(c, [&clauses](BDD bdd) -> BDD {
        return bdd *= clauses;
    });
    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
        for (const BDD b : removedVertices[level - 1]) {
            remove(c, b, level);
        }
    }
}

void BaseNSFManager::optimize(Computation &c) const {
    if (!c.isLeaf()) {
        for (Computation* cC : c.nestedSet()) {
            optimize(*cC);
        }
    }
    compressConjunctive(c);
}

//bool BaseNSFManager::split(Computation& c) const {
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
int BaseNSFManager::compressConjunctive(Computation &c) const {
    int depth = c.depth();

    if (depth == 0) {
        return 0;
    } else {
        int subsetChecksSuccessful = 0;

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
                    //                    if (optimizeNow(false)) {
                    if (*(c1) <= c2) {
                        subsetChecksSuccessful++;
                        deleteIt2 = true;
                    }
                    //                    } else if (optimizeNow(true)) {
                    if (c2 <= *(c1)) {
                        subsetChecksSuccessful++;
                        deleteIt1 = true;
                        //                        }
                    }

                } else {
                    //                    if (optimizeNow(true)) {
                    if (c2 <= *(c1)) {
                        subsetChecksSuccessful++;
                        deleteIt2 = true;
                    }
                    //                    } else if (optimizeNow(false)) {
                    if (*(c1) <= c2) {
                        subsetChecksSuccessful++;
                        deleteIt1 = true;
                    }
                    //                    }

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
        return subsetChecksSuccessful;
    }
}

const BDD BaseNSFManager::evaluateNSF(const std::vector<BDD>& cubesAtlevels, const Computation& c, bool keepFirstLevel) const {
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

    int level = c.level();
    if (level != 1 || !keepFirstLevel) {
        if (c.isExistentiallyQuantified()) {
            ret = ret.ExistAbstract(cubesAtlevels[level - 1], 0);
        } else {
            ret = ret.UnivAbstract(cubesAtlevels[level - 1]);
        }
    }

    return ret;
}
