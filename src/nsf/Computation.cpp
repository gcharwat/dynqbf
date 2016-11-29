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

#include <cuddInt.h>

#include "Computation.h"

Computation::Computation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd) {
    unsigned int level = quantifierSequence.size();
    unsigned int depth = 0;
    NSF* current = new NSF(level, depth, quantifierSequence.at(level - 1));
    current->setValue(bdd);
    while (level > 1) {
        level--;
        depth++;
        NSF* parent = new NSF(level, depth, quantifierSequence.at(level - 1));
        parent->insertNSF(current);
        current = parent;
    }
    _nsf = current;

    _variableDomain = new std::vector<BDD>();
    for (unsigned int level = 1; level <= cubesAtLevels.size(); level++) {
        _variableDomain->push_back(cubesAtLevels.at(level - 1));
    }
}

Computation::Computation(const Computation& other) {
    _nsf = new NSF(*(other._nsf));
    _variableDomain = new std::vector<BDD>(*(other._variableDomain));
}

Computation::~Computation() {
    delete _nsf;
    delete _variableDomain;
}

void Computation::apply(const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f) {
    addToVariableDomain(cubesAtLevels);
    _nsf->apply(f);
}

void Computation::apply(const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    addToVariableDomain(cubesAtLevels);
    _nsf->apply(clauses);
}

void Computation::conjunct(const Computation& other) {
    addToVariableDomain(*(other._variableDomain));
    _nsf->conjunct(*(other._nsf));
}

void Computation::remove(const BDD& variable, const unsigned int vl) {
    removeFromVariableDomain(variable, vl);
    _nsf->remove(variable, vl);
}

void Computation::remove(const std::vector<std::vector<BDD>>&removedVertices) {
    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
        for (BDD variable : removedVertices.at(level - 1)) {
            removeFromVariableDomain(variable, level);
        }
    }
    _nsf->remove(removedVertices);
}

void Computation::removeApply(const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    remove(removedVertices);
    apply(cubesAtLevels, clauses);
}

bool Computation::optimize() {
    return _nsf->optimize();
}

bool Computation::optimize(bool left) {
    return _nsf->optimize(left);
}

void Computation::sortByIncreasingSize() {
    _nsf->sortByIncreasingSize();
}

const unsigned int Computation::maxBDDsize() const {
    return _nsf->maxBDDsize();
}

const unsigned int Computation::leavesCount() const {
    return _nsf->leavesCount();
}

const unsigned int Computation::nsfCount() const {
    return _nsf->nsfCount();
}

BDD Computation::evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const {
    // assert cubesAtLevels.size() == _variableDomain->size()
    for (unsigned int level = 1; level <= _variableDomain->size(); level++) {
        if (cubesAtlevels.size() < level) {
            cubesAtlevels.push_back(_variableDomain->at(level-1));
        } else {
            cubesAtlevels.at(level - 1) *= _variableDomain->at(level - 1);
        }
    }
    return _nsf->evaluate(cubesAtlevels, keepFirstLevel);
}

bool Computation::isUnsat() const {
    return _nsf->isUnsat();
}

RESULT Computation::decide() const {
    std::vector<BDD> cubesAtlevels;
    BDD decide = evaluate(cubesAtlevels, false);
    if (decide.IsZero()) {
        return RESULT::UNSAT;
    } else if (decide.IsOne()) {
        return RESULT::SAT;
    } else {
        // decide.print(0, 2);
        return RESULT::UNDECIDED;
    }
}

BDD Computation::solutions() const {
    std::vector<BDD> cubesAtlevels;
    return evaluate(cubesAtlevels, false);
}

void Computation::print() const {
    std::cout << "Variable domain (size):" << std::endl;
    for (unsigned int level = 1; level <= _variableDomain->size(); level++) {
        std::cout << level << ": " << (_variableDomain->at(level - 1).CountPath() - 1) << std::endl;
    }
    _nsf->print();
}

void Computation::addToVariableDomain(BDD cube, const unsigned int vl) {
    _variableDomain->at(vl - 1) *= cube;
}

void Computation::addToVariableDomain(const std::vector<BDD>& cubesAtLevels) {
    for (unsigned int level = 1; level <= cubesAtLevels.size(); level++) {
        addToVariableDomain(cubesAtLevels.at(level - 1), level);
    }
}

void Computation::removeFromVariableDomain(BDD cube, const unsigned int vl) {
    (_variableDomain->at(vl - 1)) = (_variableDomain->at(vl - 1)).ExistAbstract(cube);
}

void Computation::removeFromVariableDomain(const std::vector<BDD>& cubesAtLevels) {
    for (unsigned int level = 1; level <= cubesAtLevels.size(); level++) {
        removeFromVariableDomain(cubesAtLevels.at(level - 1), level);
    }
}