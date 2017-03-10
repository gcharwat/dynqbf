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

#include "NSF.h"
#include "Computation.h"

class CacheComputation : public Computation {
public:
    CacheComputation(ComputationManager& manager, const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel);
    CacheComputation(const CacheComputation& other);

    ~CacheComputation();

    virtual void conjunct(const Computation& other) override;

    virtual void remove(const BDD& variable, const unsigned int vl) override;
    virtual void remove(const std::vector<std::vector<BDD>>&removedVertices) override;
    virtual void removeApply(const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) override;

    virtual RESULT decide() override;
    virtual BDD solutions() override;

    virtual bool optimize() override;
    virtual bool optimize(bool left) override;

    virtual void print(bool verbose) const override;
    
    unsigned int cacheSize() const;

    virtual BDD truncate(std::vector<BDD>& cubesAtlevels) override;

    // TODO: should be private but we allow access for now
    bool _keepFirstLevel;
    
protected:
    virtual BDD evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) override;

    virtual void addToRemoveCache(BDD variable, const unsigned int vl);
    void addToRemoveCache(const std::vector<std::vector<BDD>>&variables);
    void removeFromRemoveCache(BDD variable, const unsigned int vl);
    BDD popFromRemoveCache(const unsigned int vl);
    BDD popFirstFromRemoveCache(const unsigned int vl);

    bool isRemoveCacheReducible();
    virtual bool reduceRemoveCache();
    bool isRemovableAtRemoveCacheLevel(const unsigned int vl) const;
    bool isRemovableRemoveCache() const;
        
    std::vector<std::vector<BDD>>*_removeCache;
    
private:
    unsigned int _maxBDDsize;
};
