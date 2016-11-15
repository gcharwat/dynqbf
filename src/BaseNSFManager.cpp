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

#include <algorithm>

#include "Application.h"
#include "BaseNSFManager.h"
#include "Computation.h"

BaseNSFManager::BaseNSFManager(Application& app)
: app(app) {
}

BaseNSFManager::~BaseNSFManager() {
}

Computation* BaseNSFManager::newComputation(const BDD& bdd) {
    return newComputationRec(1, bdd);
}

/* TODO remove recursion */
Computation* BaseNSFManager::newComputationRec(unsigned int level, const BDD& bdd) {
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

/* TODO move to Computation.cpp */
Computation* BaseNSFManager::copyComputation(const Computation& c) {
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

void BaseNSFManager::apply(Computation& c, std::function<BDD(const BDD&)> f) {
    if (c.isLeaf()) {
        c._value = std::move(f(c.value()));
    } else {
        for (Computation* cC : c.nestedSet()) {
            apply(*cC, f);
        }
    }
}

void BaseNSFManager::apply(Computation& c, const BDD& clauses) {
    apply(c, [&clauses](BDD bdd) -> BDD {
        return bdd *= clauses;
    });
}

Computation* BaseNSFManager::conjunct(Computation& c1, Computation& c2) {
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

void BaseNSFManager::remove(Computation& c, const BDD& variable, const unsigned int vl) {
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

void BaseNSFManager::remove(Computation& c, const std::vector<std::vector<BDD>>& removedVertices) {
    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
        for (BDD b : removedVertices[level - 1]) {
            remove(c, b, level);
        }
    }
}

void BaseNSFManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) {
    apply(c, clauses);
    remove(c, removedVertices);
}

void BaseNSFManager::optimize(Computation &c) {
    if (!c.isLeaf()) {
        for (Computation* cC : c.nestedSet()) {
            optimize(*cC);
        }
    }
    compressConjunctive(c);
}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
int BaseNSFManager::compressConjunctive(Computation &c) {
    int depth = c.depth();

    if (depth == 0) {
        return 0;
    } else {
        int subsetChecksSuccessful = 0;

        std::vector<Computation*>::iterator it1;
        std::vector<Computation*>::iterator it2;
        std::vector<Computation*>::iterator end = c._nestedSet.end();
        
        for (it1 = c._nestedSet.begin(); it1 != end;) {
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
                    if (*(c1) <= c2) {
                        deleteIt2 = true;
                    } else if (c2 <= *(c1)) {
                        deleteIt1 = true;
                    }
                } else {
                    if (c2 <= *(c1)) {
                        deleteIt2 = true;
                    } else if (*(c1) <= c2) {
                        deleteIt1 = true;
                    }
                }
                if (deleteIt2) {
                    subsetChecksSuccessful++;
                    delete *it2;                    
                    end--;
                    std::iter_swap(it2, end);
                } else if (deleteIt1) {
                    subsetChecksSuccessful++;
                    delete *it1;
                    end--;
                    std::iter_swap(it1, end);
                    break;
                } else {
                    it2++;
                }
            }
            if (!deleteIt1) {
                it1++;
            }
        }
        c._nestedSet.resize(end - c._nestedSet.begin());
        return subsetChecksSuccessful;
    }
}

const BDD BaseNSFManager::evaluateNSF(const Computation& c, const std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) {
    BDD ret;
    if (c.isLeaf()) {
        ret = c.value();
    } else {
        if (c.isExistentiallyQuantified()) {
            ret = app.getBDDManager().getManager().bddZero();
            for (Computation * cC : c.nestedSet()) {
                ret += evaluateNSF(*cC, cubesAtlevels, keepFirstLevel);
            }
        } else {
            ret = app.getBDDManager().getManager().bddOne();
            for (Computation * cC : c.nestedSet()) {
                ret *= evaluateNSF(*cC, cubesAtlevels, keepFirstLevel);
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
