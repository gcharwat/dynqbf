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

#include "Application.h"

#define DYNQBF_CUDD_UNIQUE_SLOTS    CUDD_UNIQUE_SLOTS * 2
#define DYNQBF_CUDD_CACHE_SIZE      CUDD_CACHE_SLOTS * 2
#define DYNQBF_CUDD_MAXMEMORY       1024 * 1024 * 1024 * 2l

class Application;

class BDDManager {
public:
    BDDManager(Application& app);
    ~BDDManager();

    void init(unsigned int numVars);
    void init(unsigned int numVars, unsigned int numSlots, unsigned int cacheSize, unsigned long maxMemory);

    Cudd& getManager() const;

protected:
    Application& app;
    Cudd* manager;

private:
    static const std::string BDDMANAGER_SECTION;
    
    options::Option optDisableGarbageCollection;
    options::Choice optDynamicReordering;
    options::Option optPrintCUDDStats;
};
