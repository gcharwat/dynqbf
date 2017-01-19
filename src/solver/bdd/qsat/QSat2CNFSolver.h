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

#include <map>
#include "../../../Solver.h"
#include "cuddObj.hh"

namespace solver {
    namespace bdd {
        namespace qsat {

            class QSat2CNFEDMSolver : public Solver {
            public:
                QSat2CNFEDMSolver(const Application& app);

                Computation* compute(htd::vertex_t vertex) override;

                std::vector<BDD> getCubesAtLevels(htd::vertex_t currentNode) const;
                bool isUnsat(const BDD bdd);

            private:
                BDD currentClauses(htd::vertex_t currentNode);

                std::set<DdNode*> getBEntryNodes(DdNode* d) const;
                std::vector<DdNode*> getBEntryNodesRec(DdNode* node) const;
                void ddClearFlag(DdNode * f) const;
                std::set<DdNode*> getANodes(DdNode* d) const;
                std::vector<DdNode*> getANodesRec(DdNode* d) const;
                BDD removeSubsets(BDD input) const;
                BDD reduceA(BDD input) const;
                BDD getAPath(const std::vector<BDD>& aVariables, unsigned int limit, unsigned int number) const;

                void reduceB(std::set<DdNode*>& bEntryNodes) const;
                
                mutable int optimizeCounter = 0;
            };
        }
    }
} // namespace solver::bdd::qsat
