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

#include "NSF.h"
#include "Computation.h"

#pragma once

class CacheComputation : public Computation {
public:
    CacheComputation(const std::vector<NTYPE>& quantifierSequence, const BDD& bdd, unsigned int maxNSFsize, unsigned int maxBDDsize);
    CacheComputation(const CacheComputation& other);

    ~CacheComputation();

    virtual void conjunct(const Computation& other) override;

    virtual void remove(const BDD& variable, const unsigned int vl) override;
    virtual void remove(const std::vector<std::vector<BDD>>&removedVertices) override;
    virtual void removeApply(const std::vector<std::vector<BDD>>&removedVertices, const BDD& clauses) override;

    virtual const BDD evaluate(Application& app, std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const override;

    virtual void optimize() override;

    virtual void print() const override;

private:

    void addToRemoveCache(BDD variable, const unsigned int vl);
    void addToRemoveCache(const std::vector<std::vector<BDD>>&variables);
    BDD popFromRemoveCache(const unsigned int vl);
    bool isEmptyAtRemoveCacheLevel(const unsigned int vl);
    bool isEmptyRemoveCache();

    bool isRemoveCacheReducible();
    void reduceRemoveCache();

    std::vector<std::vector<BDD>>*_removeCache;
    unsigned int maxNSFsize;
    unsigned int maxBDDsize;
};
