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

#include "Application.h"

#include "BDDManager.h"

const std::string BDDManager::BDDMANAGER_SECTION = "BDD Manager";

BDDManager::BDDManager(Application& app)
: app(app)
, optDisableGarbageCollection("disable-gc", "Disable CUDD garbage collection")
, optDynamicReordering("reorder", "h", "Use dynamic BDD variable reordering heuristic <h>")
, optPrintCUDDStats("print-BDD-stats", "Print CUDD statistics") {
    app.getOptionHandler().addOption(optDisableGarbageCollection, BDDMANAGER_SECTION);
    optDynamicReordering.addChoice("none", "disable dynamic reordering");
    optDynamicReordering.addChoice("lazy-sift", "lazy sifting of variables", true);
    optDynamicReordering.addChoice("random", "randomly select adjacent variables for swapping, select best swap");
    optDynamicReordering.addChoice("random-pivot", "like random, but first variable has most nodes in BDD");
    optDynamicReordering.addChoice("sift", "for each variable (one after the another), select best position");
    optDynamicReordering.addChoice("sift-converge", "like sift, but converge");
    optDynamicReordering.addChoice("symm-sift", "symmetric sifting");
    optDynamicReordering.addChoice("symm-sift-conv", "symmetric sifting, converging");
    optDynamicReordering.addChoice("window2", "window permutation, size 2");
    optDynamicReordering.addChoice("window3", "window permutation, size 3");
    optDynamicReordering.addChoice("window4", "window permutation, size 4");
    optDynamicReordering.addChoice("window2-conv", "window permutation, size 2, converging");
    optDynamicReordering.addChoice("window3-conv", "window permutation, size 3, converging");
    optDynamicReordering.addChoice("window4-conv", "window permutation, size 4, converging");
    //optDynamicReordering.addChoice("group-sift", "");
    //optDynamicReordering.addChoice("group-sift-conv", "");
    optDynamicReordering.addChoice("annealing", "simulated annealing (tends to be slow)");
    optDynamicReordering.addChoice("genetic", "genetic algorithm (tends to be slow)");
    //optDynamicReordering.addChoice("linear", "");
    //optDynamicReordering.addChoice("linear-converge", "");
    //optDynamicReordering.addChoice("exact", "");
    app.getOptionHandler().addOption(optDynamicReordering, BDDMANAGER_SECTION);

    app.getOptionHandler().addOption(optPrintCUDDStats, BDDMANAGER_SECTION);

}

void BDDManager::init(unsigned int numVars) {
    init(numVars, DYNQBF_CUDD_UNIQUE_SLOTS, DYNQBF_CUDD_CACHE_SIZE, DYNQBF_CUDD_MAXMEMORY);
}

void BDDManager::init(unsigned int numVars, unsigned int numSlots, unsigned int cacheSize, unsigned long maxMemory) {
    manager = new Cudd(numVars, 0, numSlots, cacheSize, maxMemory);
    if (!optDisableGarbageCollection.isUsed()) {
        manager->EnableGarbageCollection();
    }
    if (optDynamicReordering.isUsed()) {
        if (optDynamicReordering.getValue() == "none") {
            manager->AutodynDisable();
        } else if (optDynamicReordering.getValue() == "random") {
            manager->AutodynEnable(CUDD_REORDER_RANDOM);
        } else if (optDynamicReordering.getValue() == "random-pivot") {
            manager->AutodynEnable(CUDD_REORDER_RANDOM_PIVOT);
        } else if (optDynamicReordering.getValue() == "sift") {
            manager->AutodynEnable(CUDD_REORDER_SIFT);
        } else if (optDynamicReordering.getValue() == "sift-converge") {
            manager->AutodynEnable(CUDD_REORDER_SIFT_CONVERGE);
        } else if (optDynamicReordering.getValue() == "symm-sift") {
            manager->AutodynEnable(CUDD_REORDER_SYMM_SIFT);
        } else if (optDynamicReordering.getValue() == "symm-sift-conv") {
            manager->AutodynEnable(CUDD_REORDER_SYMM_SIFT_CONV);
        } else if (optDynamicReordering.getValue() == "window2") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW2);
        } else if (optDynamicReordering.getValue() == "window3") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW3);
        } else if (optDynamicReordering.getValue() == "window4") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW4);
        } else if (optDynamicReordering.getValue() == "window2-conv") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW2_CONV);
        } else if (optDynamicReordering.getValue() == "window3-conv") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW3_CONV);
        } else if (optDynamicReordering.getValue() == "window4-conv") {
            manager->AutodynEnable(CUDD_REORDER_WINDOW4_CONV);
        } else if (optDynamicReordering.getValue() == "group-sift") {
            manager->AutodynEnable(CUDD_REORDER_GROUP_SIFT);
        } else if (optDynamicReordering.getValue() == "group-sift-conv") {
            manager->AutodynEnable(CUDD_REORDER_GROUP_SIFT_CONV);
        } else if (optDynamicReordering.getValue() == "annealing") {
            manager->AutodynEnable(CUDD_REORDER_ANNEALING);
        } else if (optDynamicReordering.getValue() == "genetic") {
            manager->AutodynEnable(CUDD_REORDER_GENETIC);
        } else if (optDynamicReordering.getValue() == "linear") {
            manager->AutodynEnable(CUDD_REORDER_LINEAR);
        } else if (optDynamicReordering.getValue() == "linear-converge") {
            manager->AutodynEnable(CUDD_REORDER_LINEAR_CONVERGE);
        } else if (optDynamicReordering.getValue() == "lazy-sift") {
            manager->AutodynEnable(CUDD_REORDER_LAZY_SIFT);
        } else if (optDynamicReordering.getValue() == "exact") {
            manager->AutodynEnable(CUDD_REORDER_EXACT);
        }
    } else { // default case
        manager->AutodynEnable(CUDD_REORDER_LAZY_SIFT);
    }
}

Cudd& BDDManager::getManager() const {
    return *manager;
}

BDDManager::~BDDManager() {
    if (manager != NULL) {
        if (optPrintCUDDStats.isUsed()) {
            manager->info();
        }
        delete manager;
    }
}



