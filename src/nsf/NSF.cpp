/*
Copyright 2016-2017, Guenther Charwat
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

#include <iostream>

#include "NSF.h"

NSF::NSF(unsigned int level, unsigned int depth, NTYPE type) :
_level(level),
_depth(depth),
_type(type),
_nestedSet() {
}

NSF::NSF(const NSF& other) :
_level(other._level),
_depth(other._depth),
_type(other._type),
_nestedSet() {
    if (other.isLeaf()) {
        _value = other._value;
    } else {
        for (const NSF* n : other.nestedSet()) {
            NSF* nN = new NSF(*n);
            insertNSF(nN);
        }
    }
}

NSF::~NSF() {
    for (auto& c : _nestedSet) {
        delete c;
    }
    _nestedSet.clear();
}

bool NSF::operator==(const NSF& other) const {
    if (isLeaf() && other.isLeaf()) {
        return value() == other.value();
    }
    //     XXX Fix: Condition too strong
    //        if (nestedSet().size() != other.nestedSet().size()) {
    //            return false;
    //        }

    std::vector<int> checked(other.nestedSet().size());

    int i;
    for (NSF* lc : nestedSet()) {
        bool match = false;
        i = 0;
        for (NSF* rc : other.nestedSet()) {
            if ((*lc) == (*rc)) {
                match = true;
                checked[i] = 1;
                break;
            }
            i++;
        }
        if (match == false) {
            return false;
        }
    }


    // We would like to assume that lhs and rhs only contain distinct nested computations
    // Currently, this does not hold!
    i = 0;
    for (NSF* rc : other.nestedSet()) {
        if (checked[i] != 1) {
            bool match = false;
            for (NSF* lc : nestedSet()) {
                if ((*lc) == (*rc)) {
                    match = true;
                    break;
                }
            }
            if (match == false) {
                return false;
            }
        }
        i++;
    }

    return true; // same
}

bool NSF::operator!=(const NSF& other) const {
    if (*this == other) return false;
    return true;
}

bool NSF::operator<=(const NSF& other) const {
    if (isLeaf()) {
        return value() <= other.value();
    } else {
        for (NSF* cc1 : nestedSet()) {
            bool found = false;
            for (NSF* cc2 : other.nestedSet()) {
                if ((*cc1) == (*cc2)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }
}

const BDD& NSF::value() const {
    return _value;
}

void NSF::setValue(const BDD& bdd) {
    _value = bdd;
}

const std::vector<NSF *>& NSF::nestedSet() const {
    return _nestedSet;
}

void NSF::insertNSF(NSF * nC) {
    _nestedSet.push_back(nC);
}

unsigned int NSF::depth() const {
    return _depth;
}

unsigned int NSF::level() const {
    return _level;
}

NTYPE NSF::quantifier() const {
    return _type;
}

bool NSF::isLeaf() const {
    return _depth == 0;
}

bool NSF::isExistentiallyQuantified() const {
    return _type == NTYPE::EXISTS;
}

bool NSF::isUniversiallyQuantified() const {
    return _type == NTYPE::FORALL;
}

const unsigned int NSF::maxBDDsize() const {
    if (isLeaf()) {
        return _value.nodeCount();
    } else {
        int max = 0;
        for (const auto& childComp : nestedSet()) {
            int size = childComp->maxBDDsize();
            if (size > max) {
                max = size;
            }
        }
        return max;
    }
}

const unsigned int NSF::leavesCount() const {
    if (isLeaf()) {
        return 1;
    } else {
        int counter = 0;
        for (const auto& cC : nestedSet()) {
            counter += cC->leavesCount();
        }
        return counter;
    }
}

const unsigned int NSF::nsfCount() const {
    if (isLeaf()) {
        return 1;
    } else {
        int counter = 0;
        for (const auto& cC : nestedSet()) {
            counter += cC->nsfCount();
        }
        return counter + 1;
    }
}

void NSF::print(bool verbose) const {
    if (verbose) {
        if (isExistentiallyQuantified()) std::cout << "E";
        else if (isUniversiallyQuantified()) std::cout << "A";
        else std::cout << "U";
        std::cout << " l" << _level;
        std::cout << " d" << _depth << " ";
    }
    if (isLeaf()) {
        if (value().IsZero()) {
            std::cout << "[B]";
        } else if (value().IsOne()) {
            std::cout << "[T]";
        } else {
            std::cout << "[" << _value.nodeCount() << "]";
            if (verbose) {
                std::cout << std::endl;
                _value.print(0, 2);
                std::cout << std::endl;
            }
        }
    } else {
        std::cout << "{";
        for (const auto& childComp : nestedSet()) {
            childComp->print(verbose);
        }
        std::cout << "}";
    }
}

void NSF::apply(std::function<BDD(const BDD&)> f) {
    if (isLeaf()) {
        //        _value = std::move(f(value()));
        _value = f(_value);
    } else {
        for (NSF* n : nestedSet()) {
            n->apply(f);
        }
    }
}

void NSF::apply(const BDD& clauses) {
    apply([&clauses](BDD bdd) -> BDD {
        return bdd *= clauses;
    });
}

void NSF::conjunct(const NSF& other) {
    if (isLeaf()) {
        _value *= other._value;
    } else {
        std::vector<NSF*> newNestedSet;
        newNestedSet.reserve(nestedSet().size() * other.nestedSet().size());
        for (NSF* n1 : nestedSet()) {
            for (NSF* n2 : other.nestedSet()) {
                NSF* new1 = new NSF(*n1);
                new1->conjunct(*n2);
                newNestedSet.push_back(new1);
            }
            delete n1;
        }
        _nestedSet.clear();
        _nestedSet = newNestedSet;
    }
}

void NSF::removeAbstract(const BDD& variable, const unsigned int vl) {
    if (level() == vl) {
        if (isExistentiallyQuantified()) {
            apply([&variable] (BDD b) -> BDD {
                return b.ExistAbstract(variable, 0);
            });
        } else {
            apply([&variable] (BDD b) -> BDD {
                return b.UnivAbstract(variable);
            });
        }
    } else {
        for (NSF* n : nestedSet()) {
            n->removeAbstract(variable, vl);
        }
    }
}

void NSF::remove(const BDD& variable, const unsigned int vl) {
    if (level() == vl) {
        if (isLeaf()) {
            if (isExistentiallyQuantified()) {
                _value = _value.ExistAbstract(variable, 0);
            } else {
                _value = _value.UnivAbstract(variable);
            }
        } else {
            std::vector<NSF*> newNestedSet;
            for (NSF* n : nestedSet()) {
                NSF* cop = new NSF(*n);
                n->apply([&variable] (BDD b) -> BDD {
                    return b.Restrict(variable);
                });
                cop->apply([&variable] (BDD b) -> BDD {
                    return b.Restrict(!variable);
                });
                newNestedSet.push_back(cop);
            }
            _nestedSet.insert(_nestedSet.end(), newNestedSet.begin(), newNestedSet.end());
        }
    } else {
        for (NSF* n : nestedSet()) {
            n->remove(variable, vl);
        }
    }
}

void NSF::remove(const std::vector<std::vector<BDD>>&removedVertices) {
    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
        for (BDD b : removedVertices[level - 1]) {
            remove(b, level);
        }
    }
}

bool NSF::optimize() {
    bool changed = false;
    if (!isLeaf()) {
        for (NSF* n : nestedSet()) {
            bool ret = n->optimize();
            if (ret) changed = true;
        }
    }
    if (compressConjunctive() > 0) return true;
    return changed;
}

bool NSF::optimize(bool left) {
    bool changed = false;
    if (!isLeaf()) {
        for (NSF* n : nestedSet()) {
            bool ret = n->optimize();
            if (ret) changed = true;
        }
    }
    if (left) {
        if (compressConjunctiveLeft() > 0) return true;
    } else {
        if (compressConjunctiveRight() > 0) return true;
    }
    return changed;
}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
int NSF::compressConjunctive() {
    if (depth() == 0) {
        return 0;
    } else {
        int subsetChecksSuccessful = 0;

        std::vector<NSF*>::iterator it1;
        std::vector<NSF*>::iterator it2;
        std::vector<NSF*>::iterator end = _nestedSet.end();

        for (it1 = _nestedSet.begin(); it1 != end;) {
            NSF* c1 = *it1;
            bool deleteIt1 = false;
            it2 = it1;
            it2++;
            while (it2 != end) {
                NSF& c2 = *(*it2);
                // TODO special handling for innermost quantifier
                // to be fixed when q-resolution is implemented
                bool deleteIt2 = false;
                if (depth() > 1 || isUniversiallyQuantified()) {
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
        _nestedSet.resize(end - _nestedSet.begin());
        return subsetChecksSuccessful;
    }
}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
int NSF::compressConjunctiveRight() {
    if (depth() == 0) {
        return 0;
    } else {
        int subsetChecksSuccessful = 0;

        std::vector<NSF*>::iterator it1;
        std::vector<NSF*>::iterator it2;
        std::vector<NSF*>::iterator end = _nestedSet.end();

        for (it1 = _nestedSet.begin(); it1 != end;) {
            NSF* c1 = *it1;
            bool deleteIt1 = false;
            it2 = it1;
            it2++;
            while (it2 != end) {
                NSF& c2 = *(*it2);
                // TODO special handling for innermost quantifier
                // to be fixed when q-resolution is implemented
                if (depth() > 1 || isUniversiallyQuantified()) {
                    if (c2 <= *(c1)) {
                        deleteIt1 = true;
                    }
                } else {
                    if (*(c1) <= c2) {
                        deleteIt1 = true;
                    }
                }
                if (deleteIt1) {
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
        _nestedSet.resize(end - _nestedSet.begin());
        return subsetChecksSuccessful;
    }
}

/**
 * We expect an alternating quantifier sequence!
 * 
 **/
int NSF::compressConjunctiveLeft() {
    if (depth() == 0) {
        return 0;
    } else {
        int subsetChecksSuccessful = 0;

        std::vector<NSF*>::iterator it1;
        std::vector<NSF*>::iterator it2;
        std::vector<NSF*>::iterator end = _nestedSet.end();

        for (it1 = _nestedSet.begin(); it1 != end;) {
            NSF* c1 = *it1;
            it2 = it1;
            it2++;
            while (it2 != end) {
                NSF& c2 = *(*it2);
                // TODO special handling for innermost quantifier
                // to be fixed when q-resolution is implemented
                bool deleteIt2 = false;
                if (depth() > 1 || isUniversiallyQuantified()) {
                    if (*(c1) <= c2) {
                        deleteIt2 = true;
                    }
                } else {
                    if (c2 <= *(c1)) {
                        deleteIt2 = true;
                    }
                }
                if (deleteIt2) {
                    subsetChecksSuccessful++;
                    delete *it2;
                    end--;
                    std::iter_swap(it2, end);
                } else {
                    it2++;
                }
            }
            it1++;
        }
        _nestedSet.resize(end - _nestedSet.begin());
        return subsetChecksSuccessful;
    }
}

void NSF::sortByIncreasingSize() {
    std::sort(_nestedSet.begin(), _nestedSet.end(), [] (const NSF* c1, const NSF * c2) -> bool {
        long unsigned int c1Size = (c1->isLeaf() ? c1->maxBDDsize() : c1->nestedSet().size());
        long unsigned int c2Size = (c2->isLeaf() ? c2->maxBDDsize() : c2->nestedSet().size());
        return (c1Size < c2Size);
    });
}

BDD NSF::truncate(const std::vector<BDD>& cubesAtlevels) {
    BDD ret;
    if (isLeaf()) {
        ret = value();
    } else {
        ret = nestedSet().front()->truncate(cubesAtlevels);
        for (unsigned int it = 1; it < nestedSet().size(); it++) {
            if (isExistentiallyQuantified()) {
                ret += nestedSet().at(it)->truncate(cubesAtlevels);
            } else {
                ret *= nestedSet().at(it)->truncate(cubesAtlevels);
            }
        }
    }

    if (isExistentiallyQuantified()) {
        ret = ret.ExistAbstract(cubesAtlevels[level() - 1], 0);
    } else {
        ret = ret.UnivAbstract(cubesAtlevels[level() - 1]);
    }

    return ret;
}

const BDD NSF::evaluate(const std::vector<BDD>& cubesAtlevels, const bool keepFirstLevel) {
    BDD ret;
    if (level() == 1 && keepFirstLevel) {
        if (isLeaf()) {
            ret = value();
        } else {
            ret = nestedSet().front()->evaluate(cubesAtlevels, keepFirstLevel);
            for (unsigned int it = 1; it < nestedSet().size(); it++) {
                if (isExistentiallyQuantified()) {
                    ret += nestedSet().at(it)->evaluate(cubesAtlevels, keepFirstLevel);
                } else {
                    ret *= nestedSet().at(it)->evaluate(cubesAtlevels, keepFirstLevel);
                }
            }
        }
    } else {
        if (isLeaf()) {
            ret = value();
        } else {
            ret = nestedSet().front()->evaluate(cubesAtlevels, keepFirstLevel);
        }
        if (isExistentiallyQuantified()) {
            ret = ret.ExistAbstract(cubesAtlevels[level() - 1], 0);
        } else {
            ret = ret.UnivAbstract(cubesAtlevels[level() - 1]);
        }
        
        
        for (unsigned int it = 1; it < nestedSet().size(); it++) {
            if (isExistentiallyQuantified()) {
                ret += (nestedSet().at(it)->evaluate(cubesAtlevels, keepFirstLevel).ExistAbstract(cubesAtlevels[level() - 1], 0));
            } else {
                ret *= (nestedSet().at(it)->evaluate(cubesAtlevels, keepFirstLevel).UnivAbstract(cubesAtlevels[level() - 1]));
            }
        }
    }
//std::ocut << "test 4" << std::endl;
//ret.print(0,1);
//    if (ret.IsZero()) {
//        for (NSF* child : _nestedSet) {
//            delete child;
//        }
//        _nestedSet.clear();
//
//        NSF* current = this;
//
//        // TODO handling could be shifted elsewhere
//        while (current->depth() > 0) {
//            NSF* child = new NSF(current->level() + 1, current->depth() - 1, (current->quantifier() == NTYPE::EXISTS ? NTYPE::FORALL : NTYPE::EXISTS));
//            current->insertNSF(child);
//            current = child;
//        }
//        current->setValue(ret);
//
//    }


    return ret;
}

bool NSF::isUnsat() const {

    if (isLeaf()) {
        return _value.IsZero();
    } else {
        for (const NSF* n : _nestedSet) {
            bool unsatC = n->isUnsat();
            if (isExistentiallyQuantified() && !unsatC) {
                return false;
            } else if (isUniversiallyQuantified() && unsatC) {
                return true;
            }
        }
        if (isExistentiallyQuantified()) {
            return true;
        } else {
            return false;
        }
    }
}