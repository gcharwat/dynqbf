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

#include <map>

//#include "Utils.h"
#include "Application.h"
#include "Computation.h"
//#include "options/DefaultIntegerValueOption.h"

class Application;
class Computation;

class BaseNSFManager {
public:
    BaseNSFManager(Application& app);
    virtual ~BaseNSFManager() = 0;
    
    virtual Computation* newComputation(const BDD& bdd) const;
    virtual Computation* copyComputation(const Computation& c) const;
    
    virtual void apply(Computation& c, std::function<BDD(const BDD&)> f) const;
    virtual void apply(Computation& c, const BDD& clauses) const;
    
    virtual Computation* conjunct(Computation& c1, Computation& c2) const;
    
    virtual void remove(Computation& c, const BDD& variable, const unsigned int vl) const;
    virtual void remove(Computation& c, const std::vector<std::vector<BDD>>& removedVertices) const;
    virtual void removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses) const;
    
    virtual const BDD evaluateNSF(const std::vector<BDD>& cubesAtlevels, const Computation& c, bool keepFirstLevel) const;

    virtual void optimize(Computation &c) const;

protected:
    Application& app;

    int compressConjunctive(Computation &c) const;

private:
    Computation* newComputationRec(unsigned int level, const BDD& bdd) const; // OK
};




