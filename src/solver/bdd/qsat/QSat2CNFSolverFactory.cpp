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

#include "QSat2CNFSolverFactory.h"
#include "QSat2CNFSolver.h"
#include "../../../Application.h"

#include "cuddObj.hh"
#include "cuddInt.h"
#include "cudd.h"

#include <cassert>

namespace solver {
    namespace bdd {
        namespace qsat {

            const std::string QSat2CNFSolverFactory::OPTION_SECTION = "QSAT2 (CNF) Problem options";

            QSat2CNFSolverFactory::QSat2CNFSolverFactory(Application& app, bool newDefault)
            : ::SolverFactory(app, "cnf2", "solve CNF QSAT2", newDefault) {
            }

            std::unique_ptr<::Solver> QSat2CNFSolverFactory::newSolver() const {
                
                size_t atomCount = app.getInputHypergraph()->vertexCount();
                
                MtrNode * root = Mtr_InitGroupTree(0,atomCount*2);
                (void) Mtr_MakeGroup(root,        0,2*atomCount,MTR_FIXED);
                (void) Mtr_MakeGroup(root,        0,atomCount,MTR_DEFAULT);
                (void) Mtr_MakeGroup(root,atomCount,atomCount,MTR_DEFAULT);
    
                app.getBDDManager().getManager().SetTree(root);
                
                return std::unique_ptr<::Solver>(new QSat2CNFEDMSolver(app));
            }

            BDD QSat2CNFSolverFactory::getBDDVariable(const std::string& type, const int position, const std::vector<htd::vertex_t>& vertices) const {
                if (vertices.size() > 1) {
                    throw std::runtime_error("Invalid variable call");
                }

                if (type == "d") {
                    if (position != 0) {
                        throw std::runtime_error("Invalid variable call (position for decision variable != 0)");
                    }
                    int vPos = app.getVertexOrdering().at(vertices.at(0));
                    return app.getBDDManager().getManager().bddVar(vPos);
                } else if (type == "a") {
                    if (position != 0) {
                        throw std::runtime_error("Invalid variable call (position for atom != 0)");
                    }
                    size_t atomsCount = app.getInputHypergraph()->vertexCount();
                    int vPos = app.getVertexOrdering().at(vertices.at(0));
                    return app.getBDDManager().getManager().bddVar(atomsCount + vPos);
                } else {
                    throw std::runtime_error("Invalid variable type " + type);
                }
            }

            std::vector<Variable> QSat2CNFSolverFactory::getVariables() const {
                std::vector<Variable> variables;

                for (const auto& vertex : app.getInputHypergraph()->internalGraph().vertices()) {
                    BDD var = QSat2CNFSolverFactory::getBDDVariable("d", 0,{vertex});
                    Variable v = Variable(var.getNode()->index, "d", 0, {
                        vertex
                    });
                    variables.push_back(v);
                }
                for (const auto& vertex : app.getInputHypergraph()->internalGraph().vertices()) {
                    BDD var = QSat2CNFSolverFactory::getBDDVariable("a", 0,{vertex});
                    Variable v = Variable(var.getNode()->index, "a", 0, {
                        vertex
                    });
                    variables.push_back(v);
                }

                return variables;
            }
        }
    }
} // namespace solver::bdd::qsat
