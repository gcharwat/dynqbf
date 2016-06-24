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

#include <sstream>
#include <iostream>
#include <list>

#include "QSatCNFSolver.h"
#include "../../../Application.h"
#include "../../../Printer.h"
#include "../../../AbortException.h"

#include "cuddObj.hh"
#include "htd/InducedSubgraphLabelingOperation.hpp"
#include <algorithm>

#include "../../../Utils.h"

namespace solver {
    namespace bdd {
        namespace qsat {

            QSatCNFLDMSolver::QSatCNFLDMSolver(const Application& app, bool checkUnsat)
            : ::Solver(app)
            , checkUnsat(checkUnsat) {
                std::cout << "Warning: Not tested/optimized" << std::endl;
            }

            Computation* QSatCNFLDMSolver::compute(htd::vertex_t currentNode) {

                HTDDecompositionPtr decomposition = app.getDecomposition();
                const SolverFactory& varMap = (app.getSolverFactory());
                const NSFManager& nsfMan = app.getNSFManager();

                Computation* cC = NULL;

                if (decomposition->isLeaf(currentNode)) {
                    cC = nsfMan.newComputation(1, app.getBDDManager().getManager().bddOne());
                } else {
                    bool first = true;

                    std::vector<Computation*> childComputations;
                    for (const auto& child : decomposition->children(currentNode)) {
                        childComputations.push_back(compute(child));
                    }
                    for (unsigned int childIndex = 0; childIndex < decomposition->childCount(currentNode); childIndex++) {
                        htd::vertex_t child = decomposition->children(currentNode)[childIndex];
                        Computation* tmpOuter = childComputations[childIndex];

                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables and introducing clauses");

                        // Do removal
                        const htd::ConstCollection<htd::vertex_t> forgottenVertices = decomposition->forgottenVertices(currentNode, child);

                        std::vector<std::vector < BDD >> removed(app.getNSFManager().quantifierCount());

                        for (const auto& vertex : forgottenVertices) {
                            BDD variable = varMap.getBDDVariable("a", 0,{vertex});
                            unsigned int vertexLevel = htd::accessLabel<int>(app.getInputHypergraph()->internalGraph().vertexLabel("level", vertex));
                            removed[vertexLevel - 1].push_back(variable);
                        }
                        BDD removedClauses = this->removedClauses(currentNode, child);
                        nsfMan.removeApply(*tmpOuter, removed, removedClauses);
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables and introducing clauses - done");

                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables and introducing clauses, opt");
                        nsfMan.optimize(*tmpOuter);
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables and introducing clauses, opt - done");
                        if (first) {
                            cC = tmpOuter;
                            first = false;
                        } else {
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "joining");
                            Computation * tmpJoin = nsfMan.conjunct(*cC, *tmpOuter);
                            delete cC;
                            delete tmpOuter;
                            cC = tmpJoin;
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "joining - done");
                            nsfMan.optimize(*cC);
                        }
                    }
                }

                if (checkUnsat) {
                    const std::vector<BDD> cubesAtLevels = getCubesAtLevels(currentNode);

                    app.getPrinter().solverIntermediateEvent(currentNode, *cC, "checking unsat");
                    BDD decide = nsfMan.evaluateNSF(cubesAtLevels, *cC, false);
                    app.getPrinter().solverIntermediateEvent(currentNode, *cC, "checking unsat - done");
                    if (decide == app.getBDDManager().getManager().bddZero()) {
                        throw AbortException("Intermediate unsat check successful", RESULT::UNSAT);
                    }
                }

                app.getPrinter().solverInvocationResult(currentNode, *cC);
                return cC;
            }

            const std::vector<BDD> QSatCNFLDMSolver::getCubesAtLevels(htd::vertex_t currentNode) const {
                std::vector<BDD> cubesAtLevels;
                for (unsigned int i = 0; i < app.getNSFManager().quantifierCount(); i++) {
                    cubesAtLevels.push_back(app.getBDDManager().getManager().bddOne());
                }
                const std::vector<htd::vertex_t> currentVertices = app.getDecomposition()->bagContent(currentNode);
                for (const auto v : currentVertices) {
                    int levelIndex = (htd::accessLabel<int>(this->app.getInputHypergraph()->internalGraph().vertexLabel("level", v))) - 1;
                    BDD vertexVar = app.getSolverFactory().getBDDVariable("a", 0,{v});
                    cubesAtLevels[levelIndex] *= vertexVar;
                }
                return cubesAtLevels;
            }

            BDD QSatCNFLDMSolver::removedClauses(htd::vertex_t currentNode, htd::vertex_t childNode) {
                HTDDecompositionPtr decomposition = app.getDecomposition();
                Cudd manager = app.getBDDManager().getManager();

                const SolverFactory& varMap = app.getSolverFactory();

                BDD clauses = manager.bddOne();
                // const htd::ConstCollection<htd::Hyperedge> &inducedEdges = htd::accessLabel<htd::ConstCollection < htd::Hyperedge >> (decomposition->vertexLabel(htd::IntroducedSubgraphLabelingOperation::INTRODUCED_SUBGRAPH_LABEL_IDENTIFIER, currentNode));

                const htd::FilteredHyperedgeCollection &inducedEdges = decomposition->inducedHyperedges(currentNode);
                const htd::FilteredHyperedgeCollection &childEdges = decomposition->inducedHyperedges(childNode);

                for (const auto& childEdge : childEdges) {
                    if (std::find(inducedEdges.begin(), inducedEdges.end(), childEdge) != inducedEdges.end()) {
                        continue;
                    }
                    htd::id_t edgeId = childEdge.id();
                    const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(app.getInputHypergraph()->edgeLabel("signs", edgeId));
                    BDD clause = manager.bddZero();

                    std::vector<bool>::const_iterator index = edgeSigns.begin();
                    for (const auto& vertex : childEdge) {

                        BDD vertexVar = varMap.getBDDVariable("a", 0,{vertex});
                        clause += (*index ? vertexVar : !vertexVar);
                        index++;
                    }
                    clauses *= clause;
                }
                return clauses;
            }

            bool QSatCNFLDMSolver::isUnsat(const Computation & c) {
                if (c.isLeaf()) {
                    return c.value().IsZero();
                } else {
                    for (const Computation* cC : c.nestedSet()) {
                        bool unsatC = isUnsat(*cC);
                        if (c.isExistentiallyQuantified() && !unsatC) {
                            return false;
                        } else if (c.isUniversiallyQuantified() && unsatC) { // FORALL
                            return true;
                        }
                    }
                    if (c.isExistentiallyQuantified()) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }

            RESULT QSatCNFLDMSolver::decide(const Computation & c) {
                Cudd manager = app.getBDDManager().getManager();
                NSFManager& nsfManager = app.getNSFManager();

                std::vector<BDD> cubesAtlevels;
                for (unsigned int i = 0; i < app.getNSFManager().quantifierCount(); i++) {
                    cubesAtlevels.push_back(manager.bddOne());
                }
                BDD decide = nsfManager.evaluateNSF(cubesAtlevels, c, false);
                if (decide == manager.bddZero()) {
                    return RESULT::UNSAT;
                } else if (decide == manager.bddOne()) {
                    return RESULT::SAT;
                } else {
                    decide.print(0, 2);
                    return RESULT::UNDECIDED;
                }
            }

            BDD QSatCNFLDMSolver::solutions(const Computation& c) {
                NSFManager& nsfManager = app.getNSFManager();
                std::vector<BDD> cubesAtlevels;
                for (unsigned int i = 0; i < app.getNSFManager().quantifierCount(); i++) {
                    cubesAtlevels.push_back(app.getBDDManager().getManager().bddOne());
                }
                return nsfManager.evaluateNSF(cubesAtlevels, c, true);
            }

        }
    }

} // namespace solver::bdd::qsat
