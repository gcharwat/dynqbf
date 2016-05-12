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

#include "Utils.h"
#include "Application.h"
#include "Computation.h"

class Application;
class Computation;

class NSFManager {
public:
    NSFManager(Application& app);
    ~NSFManager();

    void init(std::vector<NTYPE> quantifierSequence);

    Computation* newComputation(unsigned int level, BDD bdd) const;
    Computation* copyComputation(const Computation& c) const;

    void apply(Computation& c, std::function<BDD(const BDD&)> f) const;
    Computation* conjunct(Computation& c1, Computation& c2) const;

    void removeApply(Computation& c, std::vector<std::vector<BDD>> removedVertices, const BDD& clauses) const;

    void remove(Computation& c, const BDD& variable, const unsigned int vl) const;

    void optimize(Computation &c) const;

    const BDD evaluateNSF(const std::vector<BDD>& cubesAtlevels, const Computation& c, bool keepFirstLevel) const;

    void pushBackQuantifier(const NTYPE quantifier);
    void pushFrontQuantifier(const NTYPE quantifier);
    const NTYPE innermostQuantifier() const;
    const NTYPE quantifier(const unsigned int level) const;
    const unsigned int quantifierCount() const;

    void printStatistics() const;

protected:
    Application& app;

    bool split(Computation& c) const;
    void compressConjunctive(Computation &c) const;

private:
    void removeApplyRec(Computation& c, std::vector<std::vector<BDD>> removedVertices, BDD restrict, const BDD& clauses) const;
    void optimizeRec(Computation &c) const;
    void removeRec(Computation& c, const BDD& variable, const unsigned int vl) const;
    bool optimizeNow(bool half) const;

    std::vector<NTYPE> quantifierSequence;

    static const std::string NSFMANAGER_SECTION;

    options::Option optPrintStats;
    options::SingleValueOption optMaxNSFSize;
    options::SingleValueOption optMaxBDDSize;
    options::SingleValueOption optOptimizeInterval;
    options::Option optSortBeforeJoining;

    mutable int rotateCheck = 0;

    mutable unsigned long subsetChecks;
    mutable unsigned long subsetChecksSuccessful;

    mutable int maxNSFSizeEstimation;

};
