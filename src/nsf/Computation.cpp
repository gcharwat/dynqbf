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

Computation::Computation(const std::vector<NTYPE>& quantifierSequence, const BDD& bdd) {
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
}

Computation::Computation(const Computation& other) {
    _nsf = new NSF(*(other._nsf));
}

Computation::~Computation() {
    delete _nsf;
}

void Computation::apply(std::function<BDD(const BDD&)> f) {
    _nsf->apply(f);
}

void Computation::apply(const BDD& clauses) {
    _nsf->apply(clauses);
}

void Computation::conjunct(const Computation& other) {
    _nsf->conjunct(*(other._nsf));
}

void Computation::remove(const BDD& variable, const unsigned int vl) {
    _nsf->remove(variable, vl);
}

void Computation::remove(const std::vector<std::vector<BDD>>&removedVertices) {
    _nsf->remove(removedVertices);
}

void Computation::removeApply(const std::vector<std::vector<BDD>>&removedVertices, const BDD& clauses) {
    _nsf->removeApply(removedVertices, clauses);
}

void Computation::optimize() {
    _nsf->optimize();
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

const BDD Computation::evaluate(Application& app, std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const {
    return _nsf->evaluate(app, cubesAtlevels, keepFirstLevel);
}

bool Computation::isUnsat() const {
    return _nsf->isUnsat();
}

void Computation::print() const {
    _nsf->print();
}