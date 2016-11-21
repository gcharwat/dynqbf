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
#include "../../../Solver.h"
#include "cuddObj.hh"

namespace solver {
    namespace bdd {
        namespace qsat {

            class QSatCNFEDMSolver : public Solver {
            public:
                QSatCNFEDMSolver(const Application& app, bool checkUnsat);

                Computation* compute(htd::vertex_t vertex) override;
                RESULT decide(const Computation& c) override;
                BDD solutions(const Computation& c) override;

                std::vector<BDD> getCubesAtLevels(htd::vertex_t currentNode) const;
//                bool isUnsat(const Computation& c);

            private:
                BDD currentClauses(htd::vertex_t currentNode);
                bool checkUnsat;
            };
            
            class QSatCNFLDMSolver : public Solver {
            public:
                QSatCNFLDMSolver(const Application& app, bool checkUnsat);

                Computation* compute(htd::vertex_t vertex) override;
                RESULT decide(const Computation& c) override;
                BDD solutions(const Computation& c) override;

                std::vector<BDD> getCubesAtLevels(htd::vertex_t currentNode) const;
//                bool isUnsat(const Computation& c);

            private:
                BDD removedClauses(htd::vertex_t currentNode, htd::vertex_t childNode);
                bool checkUnsat;
            };
        }
    }
} // namespace solver::bdd::qsat
