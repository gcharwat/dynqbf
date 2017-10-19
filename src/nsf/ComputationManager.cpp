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

#include "../Application.h"
#include "ComputationManager.h"
#include "Computation.h"
#include "CacheComputation.h"
#include "../SolverFactory.h"
#include "../Utils.h"
#include "SimpleDependencyCacheComputation.h"

#ifdef DEPQBF_ENABLED
#include "StandardDependencyCacheComputation.h"
#endif

const std::string ComputationManager::NSFMANAGER_SECTION = "NSF Manager";

ComputationManager::ComputationManager(Application& app)
: app(app)
, optPrintStats("print-NSF-stats", "Print NSF Manager statistics")
, optMaxGlobalNSFSize("max-est-NSF-size", "e", "Split until the global estimated NSF size <e> is reached, -1 to disable limit", 125)
, optMaxBDDSize("max-BDD-size", "b", "Split if a BDD size exceeds <b> (may be overruled by max-est-NSF-size)", 100000)
, optOptimizeInterval("opt-interval", "o", "Optimize NSF every <o>-th computation step, 0 to disable", 100)
, optUnsatCheckInterval("unsat-check", "u", "Check for unsatisfiability (and remove unsat NSFs) after every <u>-th computation step, 0 to disable", 2)
, optSortBeforeJoining("sort-before-joining", "Sort NSFs by increasing size before joining; can increase subset check success rate")
, optDependencyScheme("dep-scheme", "d", "Use dependency scheme <d>")
, optDisableCache("disable-cache", "Disables removal cache (and sets e: -1, b: 0, d: naive)")
, maxGlobalNSFSizeEstimation(1)
, optIntervalCounter(0)
, optUnsatCheckCounter(0) {
    app.getOptionHandler().addOption(optOptimizeInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optUnsatCheckInterval, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optSortBeforeJoining, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxGlobalNSFSize, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optMaxBDDSize, NSFMANAGER_SECTION);
#ifdef DEPQBF_ENABLED
    optDependencyScheme.addChoice("dynamic", "naive for 2-QBFs, standard for other instances", true);
    optDependencyScheme.addChoice("standard", "standard dependency scheme");
    optDependencyScheme.addChoice("simple", "quantifier prefix");
    optDependencyScheme.addChoice("naive", "innermost variables");
#endif
#ifndef DEPQBF_ENABLED
    optDependencyScheme.addChoice("naive", "innermost variables", true);
    optDependencyScheme.addChoice("simple", "quantifier prefix");
#endif    
    app.getOptionHandler().addOption(optDependencyScheme, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optDisableCache, NSFMANAGER_SECTION);
    app.getOptionHandler().addOption(optPrintStats, NSFMANAGER_SECTION);
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
    if (variableCountAtLevels != NULL) {
        delete variableCountAtLevels;
    }
    printStatistics();
}

Computation* ComputationManager::newComputation(const std::vector<NTYPE>& quantifierSequence, const std::vector<BDD>& cubesAtLevels, const BDD& bdd) {
    bool keepFirstLevel = false;
    if (quantifierSequence.size() >= 1) { // && quantifierSequence.at(0) == NTYPE::EXISTS) {
        keepFirstLevel = app.enumerate() || app.modelCount();
    }
    if (keepFirstLevel) {
        if (optUnsatCheckInterval.getValue() > 0 && quantifierSequence.size() >= 1 && quantifierSequence.at(0) == NTYPE::FORALL) {
            throw std::runtime_error("Intermediate UNSAT checking must be disabled for enumeration and counting if outermost quantifier is universal");
        }
        if (optDisableCache.isUsed()) {
            throw std::runtime_error("Removal cache must be enabled for enumeration and counting");

        }
    }
    if (optDisableCache.isUsed()) {
        if ((optMaxGlobalNSFSize.isUsed() && optMaxGlobalNSFSize.getValue() != -1) ||
                (optMaxBDDSize.isUsed() && optMaxBDDSize.getValue() != 0) ||
                (optDependencyScheme.isUsed() && optDependencyScheme.getValue() != "naive")) {
            throw std::runtime_error("Cache can only be disabled if none of NSF size, BDD size, and dependency scheme are set");
        }
        optMaxGlobalNSFSize.setValue("-1");
        optMaxBDDSize.setValue("0");
        optDependencyScheme.setValue("naive");
    }

    
    
    Computation* c = NULL;
    
#ifdef DEPQBF_ENABLED
    if (optDependencyScheme.getValue() == "standard" || (optDependencyScheme.getValue() == "dynamic" && quantifierSequence.size() > 2)) {
        if (depqbf == NULL) {
            initializeDepqbf();
        }
        if (cuddToOriginalIds == NULL) {
            initializeCuddToOriginalIds();
        }
        // always return a new vector
        std::vector<std::set < htd::vertex_t>> notYetRemovedAtLevels = initializeNotYetRemovedAtLevels();

        c = new StandardDependencyCacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *depqbf, *cuddToOriginalIds, notYetRemovedAtLevels);
    }
#endif
    if (optDependencyScheme.getValue() == "simple") {
        if (variableCountAtLevels == NULL) {
            initializeVariableCountAtLevels();
        }
        c = new SimpleDependencyCacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel, *variableCountAtLevels);
    } 
    if (c == NULL) {
        if (!optDisableCache.isUsed()) {
            c = new CacheComputation(*this, quantifierSequence, cubesAtLevels, bdd, optMaxBDDSize.getValue(), keepFirstLevel);
        } else {
            c = new Computation(*this, quantifierSequence, cubesAtLevels, bdd);
        }
    }
    updateStats(*c);
    return c;
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
    
    unsigned int oldLeavesCount1 = c.leavesCount();
    unsigned int oldLeavesCount2 = other.leavesCount();
    
//    divideGlobalNSFSizeEstimation(c.leavesCount());
//    divideGlobalNSFSizeEstimation(other.leavesCount());
    
    if (optSortBeforeJoining.isUsed()) {
        c.sortByIncreasingSize();
        other.sortByIncreasingSize();
    }
    c.conjunct(other);
    divideGlobalNSFSizeEstimation(oldLeavesCount1);
    divideGlobalNSFSizeEstimation(oldLeavesCount2);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
        
    optimize(c);
}

void ComputationManager::remove(Computation& c, const BDD& variable, const unsigned int vl) {
    unsigned int oldLeavesCount = c.leavesCount();
//    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(variable, vl);
    divideGlobalNSFSizeEstimation(oldLeavesCount);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::remove(Computation& c, const std::vector<std::vector<BDD>>&removedVertices) {
    unsigned int oldLeavesCount = c.leavesCount();
//    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.remove(removedVertices);
    divideGlobalNSFSizeEstimation(oldLeavesCount);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::removeApply(Computation& c, const std::vector<std::vector<BDD>>&removedVertices, const std::vector<BDD>& cubesAtLevels, const BDD& clauses) {
    unsigned int oldLeavesCount = c.leavesCount();
//    divideGlobalNSFSizeEstimation(c.leavesCount());
    c.removeApply(removedVertices, cubesAtLevels, clauses);
    divideGlobalNSFSizeEstimation(oldLeavesCount);
    multiplyGlobalNSFSizeEstimation(c.leavesCount());
    optimize(c);
}

void ComputationManager::optimize(Computation &c) {
    updateStats(c);
    
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
    
    if (optOptimizeInterval.getValue() > 0) {
        optIntervalCounter++;
        optIntervalCounter %= optOptimizeInterval.getValue();

        if (optIntervalCounter == 0) {
            while ((maxGlobalNSFSizeEstimation < optMaxGlobalNSFSize.getValue()) || (optMaxGlobalNSFSize.getValue() <= -1)) {
                unsigned int oldLeavesCount = c.leavesCount();
//    divideGlobalNSFSizeEstimation(c.leavesCount());
                if (!(c.optimize(left))) {
                    break;
                }
                left = !left;
                divideGlobalNSFSizeEstimation(oldLeavesCount);
                multiplyGlobalNSFSizeEstimation(c.leavesCount());
            }
        }
    }
    updateStats(c);
}

bool ComputationManager::isUnsat(const Computation& c) const {
    return c.isUnsat();
}

RESULT ComputationManager::decide(Computation& c) {
    return c.decide();
}

BDD ComputationManager::solutions(Computation& c) {
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

void ComputationManager::incrementSplitCount() {
    splitCount++;
}

void ComputationManager::printStatistics() const {
    if (!optPrintStats.isUsed()) {
        return;
    }
    std::cout << "NSF (max NSF size): " << maxNSFsize << std::endl;
    std::cout << "NSF (max NSF size - BDD size): " << maxNSFsizeBDDsize << std::endl;
    std::cout << "NSF (max NSF size - domain size): " << maxNSFsizeDomainSize << std::endl;
    std::cout << "NSF (max NSF size - cache size): " << maxNSFsizeCacheSize << std::endl;
    
    std::cout << "NSF (max BDD size): " << maxBDDsize << std::endl;
    std::cout << "NSF (max BDD size - NSF size): " << maxBDDsizeNSFsize << std::endl;
    std::cout << "NSF (max BDD size - domain size): " << maxBDDsizeDomainSize << std::endl;
    std::cout << "NSF (max BDD size - cache size): " << maxBDDsizeCacheSize << std::endl;
    
    std::cout << "NSF (max cache size): " << maxCacheSize << std::endl;
    std::cout << "NSF (max cache size - NSF size): " << maxCacheSizeNSFsize << std::endl;
    std::cout << "NSF (max cache size - BDD size): " << maxCacheSizeBDDsize << std::endl;
    std::cout << "NSF (max cache size - domain size): " << maxCacheSizeDomainSize << std::endl;
     
    std::cout << "NSF (max domain size): " << maxDomainSize << std::endl;
    std::cout << "NSF (max domain size - NSF size): " << maxDomainSizeNSFsize << std::endl;
    std::cout << "NSF (max domain size - BDD size): " << maxDomainSizeBDDsize << std::endl;
    std::cout << "NSF (max domain size - cache size): " << maxDomainSizeCacheSize << std::endl;
    
    std::cout << "NSF (splits): " << splitCount << std::endl;
    std::cout << "NSF (abstractions): " << abstractCount << std::endl;
    std::cout << "NSF (internal abstractions): " << internalAbstractCount << std::endl;
    std::cout << "NSF (shifts): " << shiftCount << std::endl;
}

void ComputationManager::divideGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation /= (1.0 * value);
    if (maxGlobalNSFSizeEstimation < 1) {
        maxGlobalNSFSizeEstimation = 1;
    }
}

void ComputationManager::multiplyGlobalNSFSizeEstimation(int value) {
    maxGlobalNSFSizeEstimation *= (1.0 * value);
}

void ComputationManager::updateStats(const Computation& c) {
    if (optPrintStats.isUsed()) {

        unsigned int nsfSize = c.leavesCount();
        unsigned int bddSize = c.maxBDDsize();
        unsigned int domainSize = c.domainSize();
        unsigned int cacheSize = 0;

        try {
            // check if other contains a remove cache
            const CacheComputation& t = dynamic_cast<const CacheComputation&> (c);
            cacheSize = t.cacheSize();

            if (maxCacheSize < cacheSize) {
                maxCacheSize = cacheSize;
                maxCacheSizeNSFsize = nsfSize;
                maxCacheSizeBDDsize = bddSize;
                maxCacheSizeDomainSize = domainSize;
            }
        } catch (std::bad_cast exp) {
        }

        if (maxNSFsize < nsfSize) {
            maxNSFsize = nsfSize;
            maxNSFsizeBDDsize = bddSize;
            maxNSFsizeDomainSize = domainSize;
            maxNSFsizeCacheSize = cacheSize;
        }

        if (maxBDDsize < bddSize) {
            maxBDDsize = bddSize;
            maxBDDsizeNSFsize = nsfSize;
            maxBDDsizeDomainSize = domainSize;
            maxBDDsizeCacheSize = cacheSize;
        }
        
        if (maxDomainSize < domainSize) {
            maxDomainSize = domainSize;
            maxDomainSizeNSFsize = nsfSize;
            maxDomainSizeBDDsize = bddSize;
            maxDomainSizeCacheSize = cacheSize;
        }
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

std::vector<std::set < htd::vertex_t >> ComputationManager::initializeNotYetRemovedAtLevels() {
    std::vector<std::set < htd::vertex_t>> notYetRemovedAtLevels; // = new std::vector<std::set < htd::vertex_t >> ();

    for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
        unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
        while (notYetRemovedAtLevels.size() < vertexLevel) {
            std::set<htd::vertex_t> notYetRemoved;
            notYetRemovedAtLevels.push_back(notYetRemoved);
        }
        notYetRemovedAtLevels.at(vertexLevel - 1).insert(vertex);
    }
    return notYetRemovedAtLevels;
}
#endif

void ComputationManager::initializeVariableCountAtLevels() {
    variableCountAtLevels = new std::vector<unsigned int>();
    for (htd::vertex_t vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
        unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
        while (variableCountAtLevels->size() < vertexLevel) {
            variableCountAtLevels->push_back(0);
        }
        variableCountAtLevels->at(vertexLevel - 1) += 1;
    }
}
