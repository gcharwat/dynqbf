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

#pragma once

#include <iostream>
#include <set>
#include <list>
#include <functional>

#include "cuddObj.hh"
#include "htd/main.hpp"

#include "../BDDManager.h"
#include "../Instance.h"

class NSF {
public:
    ~NSF();

    NSF(const NSF& other);
    NSF(unsigned int level, unsigned int depth, NTYPE type); // TODO: should not be public

    bool operator==(const NSF& other) const;
    bool operator!=(const NSF& other) const;
    bool operator<=(const NSF& other) const;

    const BDD& value() const;
    void setValue(const BDD& bdd);

    const std::vector<NSF *>& nestedSet() const;
    void insertNSF(NSF * nsf);
    void removeNSF(NSF * nsf);

    unsigned int depth() const;
    unsigned int level() const;
    NTYPE quantifier() const;
    bool isLeaf() const;
    bool isExistentiallyQuantified() const;
    bool isUniversiallyQuantified() const;

    const unsigned int maxBDDsize() const;
    const unsigned int leavesCount() const;
    const unsigned int nsfCount() const;

    void print(bool verbose = false) const;

    void apply(std::function<BDD(const BDD&)> f);
    void apply(const BDD& clauses);

    void conjunct(const NSF& other);

    void removeAbstract(const BDD& variable, const unsigned int vl);
    
    void remove(const BDD& variable, const unsigned int vl);
    void remove(const std::vector<std::vector<BDD>>&removedVertices);

    // deprecated
    BDD truncate(const std::vector<BDD>& cubesAtlevels);
    const BDD evaluate(const std::vector<BDD>& cubesAtlevels, const bool keepFirstLevel);
    bool isUnsat() const;

    bool optimize();
    bool optimize(bool left);
    
    void sortByIncreasingSize();

protected:

    int compressConjunctive();
    int compressConjunctiveLeft();
    int compressConjunctiveRight();

private:
    unsigned int _level;
    unsigned int _depth;
    NTYPE _type;
    BDD _value;
    std::vector<NSF *> _nestedSet;
};


