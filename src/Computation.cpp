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

#include <iostream>

#include "Computation.h"
#include "SolverFactory.h"

Computation::Computation(unsigned int level, unsigned int depth, NTYPE type) :
_level(level),
_depth(depth),
_type(type),
_nestedSet(),
_removeCache() {
}

Computation::~Computation() {
    for (auto& c : _nestedSet) {
        delete c;
    }
    _nestedSet.clear();
    _removeCache.clear();
}

bool Computation::operator==(const Computation& other) const {
    if (isLeaf() && other.isLeaf()) {
        return value() == other.value();
    }
//     XXX Fix: Condition too strong
//        if (nestedSet().size() != other.nestedSet().size()) {
//            return false;
//        }

    std::vector<int> checked(other.nestedSet().size());

    int i;
    for (Computation* lc : nestedSet()) {
        bool match = false;
        i = 0;
        for (Computation* rc : other.nestedSet()) {
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
    for (Computation* rc : other.nestedSet()) {
        if (checked[i] != 1) {
            bool match = false;
            for (Computation* lc : nestedSet()) {
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

bool Computation::operator!=(const Computation& other) const {
    if (*this == other) return false;
    return true;
}

bool Computation::operator<=(const Computation& other) const {
    if (isLeaf()) {
        return value() <= other.value();
    } else {
        for (Computation* cc1 : nestedSet()) {
            bool found = false;
            for (Computation* cc2 : other.nestedSet()) {
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

const BDD& Computation::value() const {
    return _value;
}

void Computation::setValue(const BDD& bdd) {
    _value = bdd;
}

const std::vector<Computation *>& Computation::nestedSet() const {
    return _nestedSet;
}

void Computation::insert(Computation * nC) {
    _nestedSet.push_back(nC);
}

const std::vector<BDD>& Computation::removeCache() const {
    return _removeCache;
}

std::vector<BDD>& Computation::mutableRemoveCache() {
    return _removeCache;
}

void Computation::addToRemoveCache(BDD bdd) {
    _removeCache.push_back(bdd);
}

unsigned int Computation::depth() const {
    return _depth;
}

unsigned int Computation::level() const {
    return _level;
}

NTYPE Computation::quantifier() const {
    return _type;
}

bool Computation::isLeaf() const {
    return _depth == 0;
}

bool Computation::isExistentiallyQuantified() const {
    return _type == NTYPE::EXISTS;
}

bool Computation::isUniversiallyQuantified() const {
    return _type == NTYPE::FORALL;
}

const int Computation::maxBDDsize() const {
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

const int Computation::leavesCount() const {
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

const int Computation::nsfCount() const {
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

void Computation::print() const {
    if (nestedSet().size() == 0) {
        std::cout << std::endl;
        _value.print(0, 2);
        std::cout << std::endl;
    } else {
        std::cout << "{ ";
        for (const auto& childComp : nestedSet()) {
            childComp->print();
        }
        std::cout << "} ";
    }
}

void Computation::printCompact() const {
    if (isExistentiallyQuantified()) std::cout << "E";
    else if (isUniversiallyQuantified()) std::cout << "A";
    else std::cout << "U";
    if (isLeaf()) {
        if (value().IsZero()) {
            std::cout << "[B]";
        } else if (value().IsOne()) {
            std::cout << "[T]";
        } else {
            std::cout << "[" << _value.nodeCount() << "]";
            _value.print(0, 2);
        }
    } else {
        std::cout << "{";
        for (const auto& childComp : nestedSet()) {
            childComp->printCompact();
        }
        std::cout << "}";
    }
}
