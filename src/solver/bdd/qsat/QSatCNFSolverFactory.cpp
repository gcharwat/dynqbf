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

#include "QSatCNFSolverFactory.h"
#include "QSatCNFSolver.h"
#include "../../../Application.h"

#include "cuddObj.hh"
#include "cuddInt.h"
#include "cudd.h"

#include <cassert>

namespace solver {
    namespace bdd {
        namespace qsat {

            const std::string QSatCNFSolverFactory::OPTION_SECTION = "QSAT (CNF) Problem options";

            QSatCNFSolverFactory::QSatCNFSolverFactory(Application& app, bool newDefault)
            : ::SolverFactory(app, "cnf", "solve CNF QSAT", newDefault)
            , optUseLDM("ldm", "Use late decision method")
            , optCheckUnsat("check-unsat", "Check for unsatisfiable computations") {
                app.getOptionHandler().addOption(optUseLDM, OPTION_SECTION);
                app.getOptionHandler().addOption(optCheckUnsat, OPTION_SECTION);
            }

            std::unique_ptr<::Solver> QSatCNFSolverFactory::newSolver() const {
                if (optUseLDM.isUsed()) {
                    return std::unique_ptr<::Solver>(new QSatCNFLDMSolver(app, optCheckUnsat.isUsed()));
                } else {
                    return std::unique_ptr<::Solver>(new QSatCNFEDMSolver(app, optCheckUnsat.isUsed()));
                }
            }

            BDD QSatCNFSolverFactory::getBDDVariable(const std::string& type, const int position, const std::vector<htd::vertex_t>& vertices) const {
                if (vertices.size() > 1) {
                    throw std::runtime_error("Invalid variable call");
                }

                if (type == "a") {
                    if (position != 0) {
                        throw std::runtime_error("Invalid variable call (position for atom != 0)");
                    }
                    int vPos = app.getVertexOrdering().at(vertices.at(0));
                    return app.getBDDManager().getManager().bddVar(vPos);
                } else {
                    throw std::runtime_error("Invalid variable type " + type);
                }
            }

            std::vector<Variable> QSatCNFSolverFactory::getVariables() const {
                std::vector<Variable> variables;

                for (const auto& vertex : app.getInputInstance()->hypergraph->internalGraph().vertices()) {
                    BDD var = QSatCNFSolverFactory::getBDDVariable("a", 0,{vertex});
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
