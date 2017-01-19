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

#include "../Application.h"
#include "ComputationManager.h"
#include "Computation.h"
#include "CacheComputation.h"
#include "../SolverFactory.h"
#include "../Utils.h"
#include "SimpleDependencyCacheComputation.h"

#ifdef DEPQBF_ENABLED
#include "DependencyCacheComputation.h"
#endif

const std::string ComputationManager::NSFMANAGER_SECTION = "NSF Manager";

ComputationManager::ComputationManager(Application& app)
: app(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxGlobalNSFSize("max-est-NSF-size", "s", "Split until the global estimated NSF size <s> is reached, -1 to disable limit", 1000)
, optMaxBDDSize("max-BDD-size", "s", "Split if a BDD size exceeds <s> (may be overruled by max-est-NSF-size)", 3000)
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step, 0 to disable", 4)
, optUnsatCheckInterval("unsat-check", "i", "Check for unsatisfiability after every <i>-th NSF join, 0 to disable", 2)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, optDependencyScheme("dep-scheme", "d", "Use dependency scheme <d>")
, optTimeout("timeout", "t", "Timeout per computation iteration, 0 to disable", 0)
, maxGlobalNSFSizeEstimation(1)
, optIntervalCounter(0)
, optUnsatCheckCounter(0) {
    app.getOptionHandler().addOption(optOptimizeInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optUnsatCheckInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxGlobalNSFSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxBDDSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optSortBeforeJoining, NSFMANAGER_SECTION);
    optDependencyScheme.addChoice("none", "use no dependency scheme", true);
    optDependencyScheme.addChoice("simple", "use simple dependency scheme");
#ifdef DEPQBF_ENABLED
    optDependencyScheme.addChoice("standard", "use standard dependency scheme");
#endif
    app.getOptionHandler().addOption(optDependencyScheme, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
    
    app.getOptionHandler().addOption(optTimeout, NSFMANAGER_SECTION);
    
    beginClock = std::clock();
}

ComputationManager::~ComputationManager() {
#ifdef DEPQBF_ENABLED
    if (depqbf != NULL) {
        qdpll_delete(depqbf);
    }
    if (cuddToOriginalIds != NULL) {
        delete cuddToOriginalIds;
    }
#endif    
    if (variablesAtLevels != NULL) {
        delete variablesAtLevels;
    }
    printStatistics();
}

Computation* ComputationManager::newComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd) {
    // TODO: dynamically return Computation with or without cache
    bool keepFirstLevel = false;
    if (quantifierSequence.size() >= 1 && quantifierSequence.at(0) == NTYPE::EXISTS) {
        keepFirstLevel = app.enumerate();
    }
#ifdef DEPQBF_ENABLED
    if (optDependencyScheme.getValue() == "standard") {
        if (depqbf == NULL) {
            initializeDepqbf();
        }
        if (cuddToOriginalIds == NULL) {
            initializeCuddToOriginalIds();
        }
        // always return a new vector
        std::vector<std::set < htd::vertex_t>> alreadyAbstractedAtLevels = initializeAlreadyAbstractedAtLevels();

        return new DependencyCacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *depqbf, *cuddToOriginalIds, alreadyAbstractedAtLevels);
    } 
#endif
    if (optDependencyScheme.getValue() == "simple") {
        if (variablesAtLevels == NULL) {
            initializeVariablesAtLevels();
        }
        return new SimpleDependencyCacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *variablesAtLevels);
    } else {
        return new CacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel);
    }
    //return new Computation(quantifierSequence, cubesAtLevels, bdd);
}

Computation* ComputationManager::copyComputation(const Computation& c) {
    Computation* nC = new Computation(c);
    return nC;
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f) {
    timedOut();
    c.apply(cubesAtLevels, f);
    optimize(c);
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    timedOut();
    c.apply(cubesAtLevels, clauses);
    optimize(c);
}

void ComputationManager::conjunct(Computation& c, Computation& other) {
    timedOut();
    divideGlobalNSFSizeEstimation(c.leavesCount());
    divideGlobalNSFSizeEstimation(other.leavesCount());
    if (optSortBeforeJoining.isUsed()) {
        c.sortByIncreasingSize();
        other.sortByIncreasingSize();
    }
    c.conjunct(other);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);

    if (optUnsatCheckInterval.getValue() > 0) {
        optUnsatCheckCounter++;
        optUnsatCheckCounter %= optUnsatCheckInterval.getValue();

        if (optUnsatCheckCounter == 0) {
            RESULT result = decide(c);
            if (result == RESULT::UNSAT) {
                throw AbortException("Intermediate unsat check successful", RESULT::UNSAT);
            }
        }
    }
}

void ComputationManager::remove(Computation& c, const BDD& variable, const unsigned int vl) {
    timedOut();
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(variable, vl);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices) {
    timedOut();
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(removedVertices);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    //    for (unsigned int level = 1; level <= removedVertices.size(); level++) {
    //        for (BDD variable : removedVertices.at(level - 1)) {
    //            divideGlobalNSFSizeEstimation(c.leavesCount());
    //            c.removeApply(variable, level, cubesAtLevels, clauses);
    //            multiplyGlobalNSFSizeEstimation(c.leavesCount());
    //            optimize(c);
    //        }
    //    }
    timedOut();
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.removeApply(removedVertices, cubesAtLevels, clauses);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::optimize(Computation &c) {
    timedOut();
    if (optOptimizeInterval.getValue() > 0) {
        optIntervalCounter++;
        optIntervalCounter %= optOptimizeInterval.getValue();

        if (optIntervalCounter == 0) {
            while ((maxGlobalNSFSizeEstimation < optMaxGlobalNSFSize.getValue()) || (optMaxGlobalNSFSize.getValue() <= -1)) {
                divideGlobalNSFSizeEstimation(c.leavesCount());
                if (!(c.optimize(left))) {
                    break;
                }
                left = !left;
                multiplyGlobalNSFSizeEstimation(c.leavesCount());
            }
        }
    }
}

bool ComputationManager::isUnsat(const Computation& c) const {
    return c.isUnsat();
}

RESULT ComputationManager::decide(const Computation& c) const {
    return c.decide();
}

BDD ComputationManager::solutions(const Computation& c) const {
    return c.solutions();
}

void ComputationManager::incrementInternalAbstractCount() {
    internalAbstractCount++;
}

void ComputationManager::incrementAbstractCount() {
    abstractCount++;
}

void ComputationManager::incrementShiftCount() {
    shiftCount++;
}

void ComputationManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "NSF (abstract count): " << abstractCount << std::endl;
    std::cout << "NSF (internal abstract count): " << internalAbstractCount << std::endl;
    std::cout << "NSF (shift count): " << shiftCount << std::endl;
    //std::cout << "Number of subset checks: " << subsetChecks << std::endl;
    //std::cout << "Number of successful subset checks: " << subsetChecksSuccessful << std::endl;
    //std::cout << "Subset check success rate: " << ((subsetChecksSuccessful * 1.0) / subsetChecks)*100 << std::endl;
}

void ComputationManager::divideGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation /= value;
    if (maxGlobalNSFSizeEstimation < 1) {
        maxGlobalNSFSizeEstimation = 1;
    }
}

void ComputationManager::multiplyGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation *= value;
}

void ComputationManager::timedOut() {
    if (optTimeout.getValue() == 0) {
        return;
    }
    
    std::clock_t currentClock = std::clock();
    double elapsed_total_secs = double(currentClock - beginClock) / CLOCKS_PER_SEC;
    if (elapsed_total_secs > optTimeout.getValue()) {
        beginClock = std::clock();
        throw AbortException("Timeout", RESULT::TIMEDOUT);
    }
}

#ifdef DEPQBF_ENABLED
void ComputationManager::initializeDepqbf() {
    depqbf = qdpll_create();
    std::string depMan = "--dep-man=qdag";
    std::vector<char> depManC(depMan.begin(), depMan.end());
    depManC.push_back('\0');
    qdpll_configure(depqbf, &depManC[0]);

    for (unsigned int level = 1; level <= app.getInputInstance()->getQuantifierSequence().size(); level++) {
        NTYPE quantifier = app.getInputInstance()->quantifier(level);
        switch (quantifier) {
            case NTYPE::EXISTS:
                qdpll_new_scope(depqbf, QDPLL_QTYPE_EXISTS);
                break;
            case NTYPE::FORALL:
                qdpll_new_scope(depqbf, QDPLL_QTYPE_FORALL);
                break;
            default:
                ; //error
        }

        for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
            unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
            if (level == vertexLevel) {
                qdpll_add(depqbf, vertex);
            }
        }
        qdpll_add(depqbf, 0); // end scope
    }

    for (htd::Hyperedge edge : app.getInputInstance()->hypergraph->internalGraph().hyperedges()) {
        htd::id_t edgeId = edge.id();
        const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(app.getInputInstance()->hypergraph->edgeLabel("signs", edgeId));

        std::vector<bool>::const_iterator index = edgeSigns.begin();
        for (const auto& vertex : edge) {
            if (*index) {
                qdpll_add(depqbf, vertex);
            } else {
                qdpll_add(depqbf, -vertex);
            }
            index++;
        }
        qdpll_add(depqbf, 0); // end clause
    }

    qdpll_init_deps(depqbf);
}

void ComputationManager::initializeCuddToOriginalIds() {
    std::vector<int> htdToCuddIds = app.getVertexOrdering();
    cuddToOriginalIds = new std::vector<unsigned int>(htdToCuddIds.size() - 1, 0);

    for (unsigned int htdVertexId = 1; htdVertexId < htdToCuddIds.size(); htdVertexId++) {
        int cuddId = htdToCuddIds.at(htdVertexId);
        // TODO rename
        cuddToOriginalIds->at(cuddId) = htdVertexId;
    }
}

std::vector<std::set < htd::vertex_t>> ComputationManager::initializeAlreadyAbstractedAtLevels() {
    std::vector<std::set < htd::vertex_t>> alreadyAbstractedAtLevels; // = new std::vector<std::set < htd::vertex_t >> ();

    for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
        unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
        while (alreadyAbstractedAtLevels.size() < vertexLevel) {
            std::set<htd::vertex_t> notYetRemoved;
            alreadyAbstractedAtLevels.push_back(notYetRemoved);
        }
        alreadyAbstractedAtLevels.at(vertexLevel - 1).insert(vertex);
    }
    return alreadyAbstractedAtLevels;
}
#endif

void ComputationManager::initializeVariablesAtLevels() {
    variablesAtLevels = new std::vector<unsigned int>();
    for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
        unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
        while (variablesAtLevels->size() < vertexLevel) {
            variablesAtLevels->push_back(0);
        }
        variablesAtLevels->at(vertexLevel - 1) += 1;
    }
}