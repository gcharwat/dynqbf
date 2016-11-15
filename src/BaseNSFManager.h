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
    virtual ~BaseNSFManager();
    
    virtual Computation* newComputation(const BDD& bdd);
    virtual Computation* copyComputation(const Computation& c);
    
    virtual void apply(Computation& c, std::function<BDD(const BDD&)> f);
    virtual void apply(Computation& c, const BDD& clauses);
    
    virtual Computation* conjunct(Computation& c1, Computation& c2);
    
    virtual void remove(Computation& c, const BDD& variable, const unsigned int vl);
    virtual void remove(Computation& c, const std::vector<std::vector<BDD>>& removedVertices);
    virtual void removeApply(Computation& c, const std::vector<std::vector<BDD>>& removedVertices, const BDD& clauses);
    
    virtual const BDD evaluateNSF(const Computation& c, const std::vector<BDD>& cubesAtlevels, bool keepFirstLevel);

    virtual void optimize(Computation &c);

protected:
    Application& app;

    virtual int compressConjunctive(Computation &c);

private:
    Computation* newComputationRec(unsigned int level, const BDD& bdd); // OK
};




