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

#include <sstream>
#include <iostream>
#include <list>
#include <algorithm>

#include "QSatCNFEDMSolver.h"
#include "../../../Application.h"
#include "../../../Printer.h"
#include "../../../AbortException.h"

#include "cuddObj.hh"
#include "cuddInt.h"
#include "cudd.h"
#include "htd/InducedSubgraphLabelingOperation.hpp"

#include "../../../Utils.h"

namespace solver {
    namespace bdd {
        namespace qsat {

            QSatCNFEDMSolver::QSatCNFEDMSolver(const Application& app)
            : ::Solver(app) {
            }

            Computation* QSatCNFEDMSolver::compute(htd::vertex_t currentNode) {

                HTDDecompositionPtr decomposition = app.getDecomposition();
                const SolverFactory& varMap = (app.getSolverFactory());
                ComputationManager& nsfMan = app.getNSFManager();

                Computation* cC = NULL;

                if (decomposition->isLeaf(currentNode)) {
                    cC = nsfMan.newComputation(app.getInputInstance()->getQuantifierSequence(), getCubesAtLevels(currentNode), currentClauses(currentNode));
                } else {
                    bool first = true;

                    std::vector<Computation*> childComputations;
                    for (const auto& child : decomposition->children(currentNode)) {
                        childComputations.push_back(compute(child));
                    }
                    for (unsigned int childIndex = 0; childIndex < decomposition->childCount(currentNode); childIndex++) {
                        htd::vertex_t child = decomposition->children(currentNode)[childIndex];
                        Computation* tmpOuter = childComputations[childIndex];

                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables");

                        // Do removal
                        const htd::ConstCollection<htd::vertex_t> forgottenVertices = decomposition->forgottenVertices(currentNode, child);
                        std::vector<htd::vertex_t> forgottenVerticesSorted(forgottenVertices.begin(), forgottenVertices.end());
                        std::sort(forgottenVerticesSorted.begin(), forgottenVerticesSorted.end(), [this] (htd::vertex_t x1, htd::vertex_t x2) -> bool {
                            unsigned int vl1 = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", x1));
                            unsigned int vl2 = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", x2));
                            return (vl1 > vl2); // vertices with higher level are to be removed first
                        });

                        for (const auto& vertex : forgottenVerticesSorted) {
                            BDD variable = varMap.getBDDVariable("a", 0,{vertex});
                            unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));
                            nsfMan.remove(*tmpOuter, variable, vertexLevel);
                        }

                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables - done");

                        // Do introduction
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "introducing clauses");
                        // TODO: Only consider introduced vertices
                        BDD currentClauses = this->currentClauses(currentNode);
                        nsfMan.apply(*tmpOuter, getCubesAtLevels(currentNode), [&currentClauses](BDD bdd) -> BDD {
                            return bdd *= currentClauses;
                        });
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "introducing clauses - done");

                        if (first) {
                            cC = tmpOuter;
                            first = false;
                        } else {
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, *tmpOuter, "joining");
                            nsfMan.conjunct(*cC, *tmpOuter);
                            delete tmpOuter;
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "joining - done");
                        }
                    }
                }

                app.getPrinter().solverInvocationResult(currentNode, *cC);
                return cC;
            }

            std::vector<BDD> QSatCNFEDMSolver::getCubesAtLevels(htd::vertex_t currentNode) const {
                std::vector<BDD> cubesAtLevels;
                for (unsigned int i = 0; i < app.getInputInstance()->quantifierCount(); i++) {
                    cubesAtLevels.push_back(app.getBDDManager().getManager().bddOne());
                }
                const std::vector<htd::vertex_t> currentVertices = app.getDecomposition()->bagContent(currentNode);
                for (const auto v : currentVertices) {
                    int levelIndex = (htd::accessLabel<int>(this->app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", v))) - 1;
                    BDD vertexVar = app.getSolverFactory().getBDDVariable("a", 0,{v});
                    cubesAtLevels[levelIndex] *= vertexVar;
                }
                return cubesAtLevels;
            }

            BDD QSatCNFEDMSolver::currentClauses(htd::vertex_t currentNode) {
                HTDDecompositionPtr decomposition = app.getDecomposition();
                Cudd manager = app.getBDDManager().getManager();

                const SolverFactory& varMap = app.getSolverFactory();

                BDD clauses = manager.bddOne();

                // const htd::ConstCollection<htd::Hyperedge> &inducedEdges = htd::accessLabel<htd::ConstCollection < htd::Hyperedge >> (decomposition->vertexLabel(htd::IntroducedSubgraphLabelingOperation::INTRODUCED_SUBGRAPH_LABEL_IDENTIFIER, currentNode));

                const htd::FilteredHyperedgeCollection &inducedEdges = decomposition->inducedHyperedges(currentNode);

                for (const auto& inducedEdge : inducedEdges) {
                    htd::id_t edgeId = inducedEdge.id();
                    const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(app.getInputInstance()->hypergraph->edgeLabel("signs", edgeId));
                    BDD clause = manager.bddZero();

                    std::vector<bool>::const_iterator index = edgeSigns.begin();
                    for (const auto& vertex : inducedEdge) {

                        BDD vertexVar = varMap.getBDDVariable("a", 0,{vertex});
                        clause += (*index ? vertexVar : !vertexVar);
                        index++;
                    }
                    clauses *= clause;
                }
                return clauses;
            }
        }
    }

} // namespace solver::bdd::qsat
