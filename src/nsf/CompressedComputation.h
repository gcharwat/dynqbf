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
#include "CacheComputation.h"

#pragma once

class CompressedComputation : public CacheComputation {
public:
    CompressedComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd, unsigned int maxBDDsize, bool keepFirstLevel);
    CompressedComputation(const CompressedComputation& other);

    ~CompressedComputation();

    virtual void conjunct(const Computation& other) override;

    virtual void remove(const BDD& variable, const unsigned int vl) override;
    virtual void remove(const std::vector<std::vector<BDD>>&removedVertices) override;
    virtual void removeApply(const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) override;

    virtual RESULT decide() const override;
    virtual BDD solutions() const override;

    virtual bool optimize() override;
    virtual bool optimize(bool left) override;

    virtual void print() const override;

protected:
    virtual BDD evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel) const override;

};
