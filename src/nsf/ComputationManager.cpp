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

/**
 TODO:
 * elegant handling of when to optimize
 * elegant handling of maxBDDzie and maxNSFsize (refactor)
 * XXX: removeCache handling
 */

#include "../Application.h"
#include "ComputationManager.h"
#include "Computation.h"
#include "CacheComputation.h"
#include "DependencyCacheComputation.h"
#include "../SolverFactory.h"
#include "../Utils.h"
#include "SimpleDependencyCacheComputation.h"

const std::string ComputationManager::NSFMANAGER_SECTION = "NSF Manager";

ComputationManager::ComputationManager(Application& app)
: app(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxGlobalNSFSize("max-est-NSF-size", "s", "Split until the global estimated NSF size <s> is reached, -1 to disable limit", 1000)
, optMaxBDDSize("max-BDD-size", "s", "Split if a BDD size exceeds <s> (may be overruled by max-est-NSF-size)", 3000)
, optOptimizeInterval("opt-interval", "i", "Optimize NSF every <i>-th computation step,0 to disable", 4)
, optUnsatCheckInterval("unsat-check", "i", "Check for unsatisfiability after every <i>-th NSF join, 0 to disable", 2)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, optDependencyScheme("dep-scheme", "d", "Use dependency scheme <d>")
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
    optDependencyScheme.addChoice("standard", "use standard dependency scheme (experimental)");
    app.getOptionHandler().addOption(optDependencyScheme, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
}

ComputationManager::~ComputationManager() {
    if (depqbf != NULL) {
        qdpll_delete(depqbf);
    }
    if (cuddToOriginalIds != NULL) {
        delete cuddToOriginalIds;
    }
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

    if (optDependencyScheme.getValue() == "standard") {
        if (depqbf == NULL) {
            depqbf = qdpll_create();
            //qdpll_configure(depqbf, "--dep-man=qdag");
            std::string depMan = "--dep-man=simple";
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

            unsigned int deps = 0;
            unsigned int maxDeps = 0;
            for (htd::vertex_t v1 : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
                unsigned int vl1 = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", v1));
                for (htd::vertex_t v2 : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
                    unsigned int vl2 = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", v2));
                    if (qdpll_var_depends(depqbf, v1, v2)) {
                        deps++;
                    }
                    if (vl1 < vl2) {
                        maxDeps++;
                    }

                }
            }
            std::cout << "Dependencies: " << deps << " / " << maxDeps << "\t\t" << (maxDeps - deps) << std::endl;

            if (cuddToOriginalIds == NULL) {
                std::vector<int> htdToCuddIds = app.getVertexOrdering();
                cuddToOriginalIds = new std::vector<unsigned int>(htdToCuddIds.size() - 1, 0);

                for (unsigned int htdVertexId = 1; htdVertexId < htdToCuddIds.size(); htdVertexId++) {
                    int cuddId = htdToCuddIds.at(htdVertexId);
                    //                std::string vertexName = app.getInputInstance()->hypergraph->vertexName(htdVertexId);
                    //                unsigned int originalId = utils::strToInt(vertexName, vertexName);
                    // TODO rename
                    cuddToOriginalIds->at(cuddId) = htdVertexId;
                }

                //        if (variables == NULL) {
                //            variables = new std::vector<Variable>(app.getSolverFactory().getVariables());
                //            std::list<Variable> variablesList(variables->begin(), variables->end());
                //            variablesList.sort([this] (Variable v1, Variable v2) -> bool {
                //                int ind1 = app.getVertexOrdering()[v1.getVertices()[0]];
                //                int ind2 = app.getVertexOrdering()[v2.getVertices()[0]];
                //                v1.
                //                return (ind1 < ind2);
                //            });
                //        }

                //            std::cout << "htd-id : " << "cudd-id" << std::endl;
                //            for (unsigned int index = 0; index < htdToCuddIds.size(); index++) {
                //                std::cout << index << ": " << htdToCuddIds[index] << std::endl;
                //            }
                //
                //            std::cout << "cudd-id : " << "original-id" << std::endl;
                //            for (unsigned int index = 0; index < cuddToOriginalIds->size(); index++) {
                //                std::cout << index << ": " << cuddToOriginalIds->at(index) << std::endl;
                //            }
            }
        }
        return new DependencyCacheComputation(quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *depqbf, *cuddToOriginalIds);
    } else if (optDependencyScheme.getValue() == "simple") {

        if (variablesAtLevels == NULL) {
            variablesAtLevels = new vector<unsigned int>(quantifierSequence.size(), 0);

            for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
                unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
                variablesAtLevels->at(vertexLevel-1) += 1;
            }
        }

        return new SimpleDependencyCacheComputation(quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *variablesAtLevels);
    } else {
        return new CacheComputation(quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel);
    }
    //return new Computation(quantifierSequence, cubesAtLevels, bdd);
}

Computation* ComputationManager::copyComputation(const Computation& c) {
    Computation* nC = new Computation(c);
    return nC;
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, std::function<BDD(const BDD&)> f) {
    c.apply(cubesAtLevels, f);
    optimize(c);
}

void ComputationManager::apply(Computation& c, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    c.apply(cubesAtLevels, clauses);
    optimize(c);
}

void ComputationManager::conjunct(Computation& c, Computation& other) {
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
    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(variable, vl);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices) {
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

    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.removeApply(removedVertices, cubesAtLevels, clauses);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::optimize(Computation &c) {
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

void ComputationManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "*** NSF Manager statistics ***" << std::endl;
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