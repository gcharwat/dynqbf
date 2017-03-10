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
//#include "ComputationManager.h"

class Computation {
public:
    Computation(ComputationManager& manager, const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd);
    Computation(const Computation& other);

    virtual ~Computation();

    virtual void apply(const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f);
    virtual void apply(const std::vector<BDD>& cubesAtLevels, const BDD& clauses);

    virtual void conjunct(const Computation& other);

    virtual void removeAbstract(const BDD& variable, const unsigned int vl);
    
    virtual void remove(const BDD& variable, const unsigned int vl);
    virtual void remove(const std::vector<std::vector<BDD>>& removedVertices);
    virtual void removeApply(const std::vector<std::vector<BDD>>& removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses);

    virtual bool isUnsat() const;
    virtual RESULT decide();
    virtual BDD solutions();

    virtual bool optimize();
    virtual bool optimize(bool left);
    void sortByIncreasingSize();

    const unsigned int maxBDDsize() const;
    const unsigned int leavesCount() const;
    const unsigned int nsfCount() const;

    virtual void print(bool verbose) const;
    
    unsigned int domainSize() const;

    virtual BDD truncate(std::vector<BDD>& cubesAtlevels);

protected:
    virtual BDD evaluate(std::vector<BDD>& cubesAtlevels, bool keepFirstLevel);
    
    ComputationManager& manager;
    NSF* _nsf;

    const std::vector<BDD>* getVariableDomain();
    void removeFromVariableDomain(BDD cube, const unsigned int vl);
    virtual void shiftVariableLevel(BDD cube, const unsigned int from, const unsigned int to);

private:
        
    void addToVariableDomain(BDD cube, const unsigned int vl);
    void addToVariableDomain(const std::vector<BDD>& cubesAtLevels);
    void removeFromVariableDomain(const std::vector<BDD>& cubesAtLevels);
    
    std::vector<BDD>* _variableDomain;
};