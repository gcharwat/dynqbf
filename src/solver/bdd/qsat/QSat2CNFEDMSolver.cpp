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

#include "QSat2CNFSolver.h"
#include "../../../Application.h"
#include "../../../Printer.h"
#include "../../../AbortException.h"

#include "cuddObj.hh"
#include "cuddInt.h"
#include "cudd.h"
#include "cuddInteract.c"
#include "htd/InducedSubgraphLabelingOperation.hpp"
#include <algorithm>

#include "../../../Utils.h"

#include <math.h>

namespace solver {
    namespace bdd {
        namespace qsat {

            QSat2CNFEDMSolver::QSat2CNFEDMSolver(const Application& app)
            : ::Solver(app) {
                std::cerr << "Warning: Not tested/optimized (development in progress)" << std::endl;
            }

            Computation* QSat2CNFEDMSolver::compute(htd::vertex_t currentNode) {

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

                        const htd::ConstCollection<htd::vertex_t> forgottenVertices = decomposition->forgottenVertices(currentNode, child);

                        for (const auto& vertex : forgottenVertices) {
                            BDD variable = varMap.getBDDVariable("a", 0,{vertex});
                            BDD decision = varMap.getBDDVariable("d", 0,{vertex});
                            unsigned int vertexLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->internalGraph().vertexLabel("level", vertex));

                            if (vertexLevel == 2) {
                                nsfMan.remove(*tmpOuter, variable, vertexLevel);
                            } else if (vertexLevel == 1) {
                                nsfMan.apply(*tmpOuter, getCubesAtLevels(currentNode), [&variable, &decision] (const BDD b) -> BDD {
                                    // TODO: Could also be done by renaming all removed variables at once
                                    return (b.Restrict(!variable) * !decision) + (b.Restrict(variable) * decision);
                                });
                            } else {
                                throw std::runtime_error("Invalid number of quantifiers");
                            }
                        }

                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "removing variables - done");

                        // Do introduction
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "introducing clauses");
                        BDD currentClauses = this->currentClauses(currentNode);
                        nsfMan.apply(*tmpOuter, getCubesAtLevels(currentNode), [&currentClauses](BDD bdd) -> BDD {
                            return bdd *= currentClauses;
                        });
                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "introducing clauses - done");
                        //                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "optimizing");
                        //                        nsfMan.optimize(*tmpOuter);
                        //                        app.getPrinter().solverIntermediateEvent(currentNode, *tmpOuter, "optimizing - done");

                        if (first) {
                            cC = tmpOuter;
                            first = false;
                        } else {
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, *tmpOuter, "joining");
                            nsfMan.conjunct(*cC, *tmpOuter);
                            delete tmpOuter;
                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "joining - done");
                            //                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "optimizing");
                            //                            nsfMan.optimize(*cC);
                            //                            app.getPrinter().solverIntermediateEvent(currentNode, *cC, "optimizing - done");
                        }
                    }
                }

                //                app.getPrinter().solverIntermediateEvent(currentNode, *cC, "reduceA");
                //                nsfMan.apply(*cC, [this](BDD bdd) -> BDD {
                //                    return reduceA(bdd);
                //                });
                //
                //                app.getPrinter().solverIntermediateEvent(currentNode, *cC, "reduceA - done");


                optimizeCounter++;
                if (optimizeCounter % 1 == 0) {
                    app.getPrinter().solverIntermediateEvent(currentNode, *cC, "subsets");
                    nsfMan.apply(*cC, getCubesAtLevels(currentNode), [this](BDD bdd) -> BDD {
                        return removeSubsets(bdd);
                    });
                    app.getPrinter().solverIntermediateEvent(currentNode, *cC, "subsets - done");
                }


                //                if (checkUnsat) {
//                std::vector<BDD> cubesAtLevels = getCubesAtLevels(currentNode);

                app.getPrinter().solverIntermediateEvent(currentNode, *cC, "checking unsat");
                RESULT decide = nsfMan.decide(*cC);
                app.getPrinter().solverIntermediateEvent(currentNode, *cC, "checking unsat - done");
                if (decide == RESULT::UNSAT) {
                    throw AbortException("Intermediate unsat check successful", RESULT::UNSAT);
                }
                //                }

                app.getPrinter().solverInvocationResult(currentNode, *cC);
                return cC;
            }

            std::set<DdNode*> QSat2CNFEDMSolver::getBEntryNodes(DdNode* node) const {
//                std::cout << "foo" << std::endl;
                std::vector<DdNode*> entryNodesVector = getBEntryNodesRec(node);
                set<DdNode*> s;
                unsigned size = entryNodesVector.size();
                for (unsigned i = 0; i < size; ++i) {
                    s.insert(entryNodesVector[i]);
                }

                //                ddDagInt(Cudd_Regular(node));
                ddClearFlag(Cudd_Regular(node));
//std::cout << s.size() << " bar" << std::endl;
                return s;
            }

            //            static int
            //            ddDagInt(
            //                    DdNode * n) {
            //                int tval, eval;
            //
            //                if (Cudd_IsComplement(n->next)) {
            //                    return (0);
            //                }
            //                n->next = Cudd_Not(n->next);
            //                if (cuddIsConstant(n)) {
            //                    return (1);
            //                }
            //                tval = ddDagInt(cuddT(n));
            //                eval = ddDagInt(Cudd_Regular(cuddE(n)));
            //                return (1 + tval + eval);
            //
            //            } /* end of ddDagInt */

            void QSat2CNFEDMSolver::ddClearFlag(DdNode * f) const {
                if (!Cudd_IsComplement(f->next)) {
                    return;
                }
                /* Clear visited flag. */
                f->next = Cudd_Regular(f->next);
                if (cuddIsConstant(f)) {
                    return;
                }
                ddClearFlag(cuddT(f));
                ddClearFlag(Cudd_Regular(cuddE(f)));
                return;

            } /* end of ddClearFlag */

            std::vector<DdNode*> QSat2CNFEDMSolver::getBEntryNodesRec(DdNode* node) const {
                std::vector<DdNode*> entryNodes;
                
                DdNode* N = Cudd_Regular(node);
                
                if (Cudd_IsComplement(N->next)) {
                    return entryNodes;
                }
                N->next = Cudd_Not(N->next);
                
                if (cuddIsConstant(N)) {
                    entryNodes.push_back(node);
                    return entryNodes;
                }

                size_t atomCount = app.getInputInstance()->hypergraph->vertexCount();

                if (N->index < atomCount) {

                    DdNode* Nv = cuddT(N);
                    DdNode* Nnv = cuddE(N);
                    if (Cudd_IsComplement(node)) {
                        Nv = Cudd_Not(Nv);
                        Nnv = Cudd_Not(Nnv);
                    }

                    entryNodes = getBEntryNodesRec(Nv);
                    //                    entryNodes.insert(top.begin(), top.end());

                    std::vector<DdNode*> bottom = getBEntryNodesRec(Nnv);
                    // remove duplicates
                    //                    for (DdNode* b : bottom) {
                    //                        bool eraseB = false;
                    //                        for (DdNode* t : top) {
                    //                            if (Cudd_bddLeq(app.getBDDManager().getManager().getManager(), b, t)) {
                    //                                top.erase(t);
                    //                                break;
                    //                            }
                    //                            if (Cudd_bddLeq(app.getBDDManager().getManager().getManager(), t, b)) {
                    //                                eraseB = true;
                    //                                break;
                    //                            }
                    //                        }
                    //                        if (!eraseB) {
                    //                            entryNodes.insert(b);
                    //                        }
                    //                    }
                    entryNodes.insert(entryNodes.end(), bottom.begin(), bottom.end());
                } else {
                    entryNodes.insert(entryNodes.end(), node);
                }
                return entryNodes;
            }

            std::set<DdNode*> QSat2CNFEDMSolver::getANodes(DdNode* f) const {
                std::vector<DdNode*> aNodesVector = getANodesRec(f);
                set<DdNode*> s;
                unsigned size = aNodesVector.size();
                for (unsigned i = 0; i < size; ++i) s.insert(aNodesVector[i]);
                return s;
            }

            std::vector<DdNode*> QSat2CNFEDMSolver::getANodesRec(DdNode* f) const {
                std::vector<DdNode*> aNodes;

                DdNode* g = Cudd_Regular(f);
                if (cuddIsConstant(g)) {
                    return aNodes;
                }
                size_t atomCount = app.getInputInstance()->hypergraph->vertexCount();


                if (g->index < atomCount) {
                    aNodes.push_back(g);

                    DdNode* n = cuddT(g);
                    if (!cuddIsConstant(n)) {
                        std::vector<DdNode*> top = getANodesRec(n);
                        aNodes.insert(aNodes.end(), top.begin(), top.end());
                    }

                    n = cuddE(g);
                    DdNode* N = Cudd_Regular(n);
                    if (!cuddIsConstant(N)) {
                        std::vector<DdNode*> bottom = getANodesRec(n);
                        aNodes.insert(aNodes.end(), bottom.begin(), bottom.end());
                    }

                }

                return aNodes;
            }

            BDD QSat2CNFEDMSolver::removeSubsets(BDD input) const {
                std::set<DdNode*> bEntryNodes = getBEntryNodes(input.getNode());

                std::list<DdNode*> list(bEntryNodes.begin(), bEntryNodes.end());

                std::list<DdNode*>::iterator it1;
                std::list<DdNode*>::iterator it2;

                unsigned int comp = 0, del = 0;
                
                std::list<DdNode*>::const_iterator end = list.end();
                for (it1 = list.begin(); it1 != end;) {
                    DdNode* c1 = *it1;
                    BDD b1(app.getBDDManager().getManager(), c1);
                    bool deleteIt1 = false;
                    it2 = it1;
                    it2++;
                    while (it2 != end) {
                        DdNode& c2 = *(*it2);
                        BDD b2(app.getBDDManager().getManager(), &c2);
                        bool deleteIt2 = false;

                        comp++;
                        if (b1 <= b2) {
                            del++;
                            deleteIt2 = true;
//                            std::cout << "del1" << std::endl;
                        } else {
                            comp++;
                        if (b2 <= b1) {
                            del++;
                            deleteIt1 = true;
//                            std::cout << "del2" << std::endl;
                        }
                        }
                        if (deleteIt2) {
                            //delete *it2;
                            it2 = list.erase(it2);
                            end = list.end();
                        } else if (deleteIt1) {
                            //delete *it1;
                            it1 = list.erase(it1);
                            c1 = *it1;
                            end = list.end();
                            break;
                        } else {
                            it2++;
                        }
                    }
                    if (!deleteIt1) {
                        it1++;
                    }
                }
                
                std::cout << "comp: " << comp << " del: " << del << std::endl;
                
                std::set<DdNode*> aNodes = getANodes(input.getNode());

                std::set<DdHalfWord> aVariableIds;
                for (DdNode* aNode : aNodes) {
                    aVariableIds.insert(aNode->index); // Cudd_Regular?
                }
                std::vector<BDD> aVariables;
                for (DdHalfWord index : aVariableIds) {
                    aVariables.push_back(app.getBDDManager().getManager().bddVar(index));
                }
                sort(aVariables.begin(), aVariables.end(),
                        [](const BDD & a, const BDD & b) -> bool {
                            return a.getNode()->index < b.getNode()->index;
                        });
                unsigned int requiredAVariables = ceil(log2(list.size()));
                unsigned int actualAVariables = aVariables.size();
                                std::cout << "|requiredAVariables| = " << requiredAVariables << std::endl;
                                std::cout << "|actualAVariables| = " << actualAVariables << std::endl;

                //                input.print(0, 5);

                if (actualAVariables > requiredAVariables) {

                    BDD result = app.getBDDManager().getManager().bddZero();
                    unsigned int i = 0;
                    for (DdNode* bEntryNode : list) {
                        BDD b(app.getBDDManager().getManager(), bEntryNode);
                        BDD aP = getAPath(aVariables, requiredAVariables, i);
                        result += b*aP;
                        i++;
                    }
                    unsigned int paddingMax = 1 << requiredAVariables;
                    while (i < paddingMax) {
                        BDD aP = getAPath(aVariables, requiredAVariables, i);
                        result += aP;
                        i++;
                    }

                    //                    std::cout << "Reduced: " << std::endl;
                    //                    result.print(0, 5);
                    return result;
                }
                return input;
            }

            BDD QSat2CNFEDMSolver::reduceA(BDD input) const {
                //                input.print(0, 5);

                std::set<DdNode*> aNodes = getANodes(input.getNode());
                std::set<DdNode*> bEntryNodes = getBEntryNodes(input.getNode());

                //                for(DdNode* b : bEntryNodes) {
                //                    BDD bdd(app.getBDDManager().getManager(), b);
                //                    bdd.print(0,5);
                //                }

                //                std::cout << "|A-nodes| = " << aNodes.size() << std::endl;
                //                std::cout << "|B-entry-nodes| = " << bEntryNodes.size() << std::endl;

                std::set<DdHalfWord> aVariableIds;
                for (DdNode* aNode : aNodes) {
                    aVariableIds.insert(aNode->index); // Cudd_Regular?
                }
                std::vector<BDD> aVariables;
                for (DdHalfWord index : aVariableIds) {
                    aVariables.push_back(app.getBDDManager().getManager().bddVar(index));
                }
                sort(aVariables.begin(), aVariables.end(),
                        [](const BDD & a, const BDD & b) -> bool {
                            return a.getNode()->index < b.getNode()->index;
                        });
                unsigned int requiredAVariables = ceil(log2(bEntryNodes.size()));
                unsigned int actualAVariables = aVariables.size();
                //                std::cout << "|requiredAVariables| = " << requiredAVariables << std::endl;
                //                std::cout << "|actualAVariables| = " << actualAVariables << std::endl;

                //                input.print(0, 5);

                if (actualAVariables > requiredAVariables) {

                    BDD result = app.getBDDManager().getManager().bddZero();
                    unsigned int i = 0;
                    for (DdNode* bEntryNode : bEntryNodes) {
                        BDD b(app.getBDDManager().getManager(), bEntryNode);
                        BDD aP = getAPath(aVariables, requiredAVariables, i);
                        result += b*aP;
                        i++;
                    }
                    unsigned int paddingMax = 1 << requiredAVariables;
                    while (i < paddingMax) {
                        BDD aP = getAPath(aVariables, requiredAVariables, i);
                        result += aP;
                        i++;
                    }

                    //                    std::cout << "Reduced: " << std::endl;
                    //                    result.print(0, 5);
                    return result;
                }
                return input;
            }

            BDD QSat2CNFEDMSolver::getAPath(const std::vector<BDD>& aVariables, unsigned int limit, unsigned int number) const {
                BDD aPath = app.getBDDManager().getManager().bddOne();

                for (unsigned int pos = 0; pos < limit; pos++) {
                    int mask = 1 << pos;
                    int masked_n = number & mask;
                    int thebit = masked_n >> pos;
                    aPath *= thebit ? !aVariables[pos] : aVariables[pos];
                }
                return aPath;
            }

//            RESULT QSat2CNFEDMSolver::decide(const Computation & c) {
//                Cudd manager = app.getBDDManager().getManager();
//                ComputationManager& nsfManager = app.getNSFManager();
//
//                std::vector<BDD> cubesAtlevels;
//                for (unsigned int i = 0; i < app.getInputInstance()->quantifierCount(); i++) {
//                    cubesAtlevels.push_back(manager.bddOne());
//                }
//                BDD decide = nsfManager.evaluate(c, cubesAtlevels, false);
//                std::set<DdNode*> bEntryNodes = getBEntryNodes(decide.getNode());
//
//                bool undecided = false;
//                for (DdNode* node : bEntryNodes) {
//                    if (node == manager.bddZero().getNode()) {
//                        return RESULT::UNSAT;
//                    } else if (!Cudd_IsConstant(node)) {
//                        undecided = true;
//                    }
//                }
//                if (undecided) {
//                    //                    decide.print(0, 5);
//                    return RESULT::UNDECIDED;
//                }
//                return RESULT::SAT;
//            }
//
//            BDD QSat2CNFEDMSolver::solutions(const Computation & c) {
//                // TODO
//                ComputationManager& nsfManager = app.getNSFManager();
//                std::vector<BDD> cubesAtlevels;
//                for (unsigned int i = 0; i < app.getInputInstance()->quantifierCount(); i++) {
//                    cubesAtlevels.push_back(app.getBDDManager().getManager().bddOne());
//                }
//                return nsfManager.evaluate(c, cubesAtlevels, true);
//            }

            BDD QSat2CNFEDMSolver::currentClauses(htd::vertex_t currentNode) {
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

            std::vector<BDD> QSat2CNFEDMSolver::getCubesAtLevels(htd::vertex_t currentNode) const {
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

            bool QSat2CNFEDMSolver::isUnsat(const BDD bdd) {
                //                BDD bdd = c.nestedSet()[0]->value();
                std::set<DdNode*> bEntryNodes = getBEntryNodes(bdd.getNode());

                for (DdNode* node : bEntryNodes) {
                    if (Cudd_IsConstant(node) && Cudd_IsComplement(node)) {
                        return true;
                    }
                }
                return false;
            }

        }
    }
} // namespace solver::bdd::qsat
