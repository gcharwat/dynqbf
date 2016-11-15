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

#pragma once

#include <iostream>
#include <set>
#include <list>
#include <functional>

#include "cuddObj.hh"
#include "htd/main.hpp"

#include "Application.h"
#include "BDDManager.h"
#include "Instance.h"

class Computation {
    friend class NSFManager; // TODO remove
    friend class BaseNSFManager;
public:
    ~Computation();

    bool operator==(const Computation& other) const;
    bool operator!=(const Computation& other) const;
    bool operator<=(const Computation& other) const;

    const BDD& value() const;

    const std::vector<Computation *>& nestedSet() const;
    
    unsigned int depth() const;
    unsigned int level() const;
    NTYPE quantifier() const;
    bool isLeaf() const;
    bool isExistentiallyQuantified() const;
    bool isUniversiallyQuantified() const;

    const int maxBDDsize() const;
    const int leavesCount() const;
    const int nsfCount() const;

    void print() const;
    void printCompact() const;

protected:
    Computation(unsigned int level, unsigned int depth, NTYPE type);
    
    void setValue(const BDD& bdd);
    
    void insert(Computation * computation);
    void remove(Computation * computation);
    
private:
    unsigned int _level;
    unsigned int _depth;
    NTYPE _type;
    BDD _value;
    std::vector<Computation *> _nestedSet;
};


