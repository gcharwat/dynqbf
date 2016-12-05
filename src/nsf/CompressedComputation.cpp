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

#include "CompressedComputation.h"

CompressedComputation::CompressedComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel)
: CacheComputation(quantifierSequence, cubesAtLevels, bdd, maxBDDsize, keepFirstLevel) {
}

CompressedComputation::CompressedComputation(const CompressedComputation& other)
: CacheComputation(other) {
}

CompressedComputation::~CompressedComputation() {
}

void CompressedComputation::conjunct(const Computation& other) {
    CacheComputation::conjunct(other);
}

void CompressedComputation::remove(const BDD& variable, const unsigned int vl) {
    CacheComputation::remove(variable, vl);
}

void CompressedComputation::remove(const std::vector<std::vector<BDD>>&removedVertices) {
    CacheComputation::remove(removedVertices);
}

void CompressedComputation::removeApply(const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    CacheComputation::removeApply(removedVertices, cubesAtLevels, clauses);
}

BDD CompressedComputation::evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const {
    return CacheComputation::evaluate(cubesAtlevels, keepFirstLevel);
}

RESULT CompressedComputation::decide() const {
    return CacheComputation::decide();
}

BDD CompressedComputation::solutions() const {
    return CacheComputation::solutions();
}

bool CompressedComputation::optimize() {
    return CacheComputation::optimize();
}

bool CompressedComputation::optimize(bool left) {
    return CacheComputation::optimize(left);
}

void CompressedComputation::print() const {
    std::cout << "Compressed Computation:" << std::endl;
    
    CacheComputation::print();
}
