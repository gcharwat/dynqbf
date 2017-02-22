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

#include <cassert>

#include "UnitLiteralPreprocessor.h"
#include "../Application.h"

#include <htd/main.hpp>
#include <limits.h>


namespace preprocessor {

    UnitLiteralPreprocessor::UnitLiteralPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "unit-literal", "apply unit literal elimination", newDefault) {
    }

    InstancePtr UnitLiteralPreprocessor::preprocess(const InstancePtr& instance) const {
        std::vector<htd::vertex_t> unitLiteralsPos;
        std::vector<htd::vertex_t> unitLiteralsNeg;

        for (auto clause : instance->hypergraph->internalGraph().hyperedges()) {
            const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(instance->hypergraph->edgeLabel("signs", clause.id()));
            
            bool unit = false;

            htd::vertex_t existentialVariable = htd::Vertex::UNKNOWN;
            bool existentialVariableSign = false;
                    
            unsigned int existsLevel = 0;
            unsigned int minForallLevel = UINT_MAX;

            for (unsigned int i = 0; i < clause.size(); i++) {
                unsigned int variableLevel = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", clause[i]));
                NTYPE quantor = instance->quantifier(variableLevel);
                if (quantor == NTYPE::EXISTS) {
                    if (existentialVariable == htd::Vertex::UNKNOWN) {
                        existentialVariable = clause[i];
                        existentialVariableSign = edgeSigns[i];
                        existsLevel = variableLevel;
                        unit = true;
                    } else {
                        unit = false;
                        break; // more than 1 existential variable in clause
                    }
                } else {
                    if (variableLevel < minForallLevel) {
                        minForallLevel = variableLevel;
                    }
                }
                if (minForallLevel < existsLevel) {
                    unit = false;
                    break; // universal is at left of exists
                }
            }
            if (unit) {
                if (existentialVariableSign)
                    unitLiteralsPos.push_back(existentialVariable);
                else 
                    unitLiteralsNeg.push_back(existentialVariable);
            }
        }

        std::sort(unitLiteralsPos.begin(), unitLiteralsPos.end());
        unitLiteralsPos.erase(std::unique(unitLiteralsPos.begin(), unitLiteralsPos.end()), unitLiteralsPos.end());
        std::sort(unitLiteralsNeg.begin(), unitLiteralsNeg.end());
        unitLiteralsNeg.erase(std::unique(unitLiteralsNeg.begin(), unitLiteralsNeg.end()), unitLiteralsNeg.end());

//        std::cout << "Unit: ";
//        for (auto variable : unitLiterals) {
//            std::cout << instance->vertexName(variable) << " ";
//        }
//        std::cout << std::endl;

        InstancePtr preprocessed(new Instance(app));
        for (const auto q : instance->getQuantifierSequence()) {
            preprocessed->pushBackQuantifier(q);
        }
        
        for (const vertexNameType vertex : instance->hypergraph->vertices()) {
            htd::vertex_t vertexId = instance->hypergraph->lookupVertex(vertex);
            if ((std::find(unitLiteralsPos.begin(), unitLiteralsPos.end(), vertexId) == unitLiteralsPos.end()) && 
                (std::find(unitLiteralsNeg.begin(), unitLiteralsNeg.end(), vertexId) == unitLiteralsNeg.end())) {
                preprocessed->hypergraph->addVertex(vertex);
                int vertexLevel = htd::accessLabel<int>(instance->hypergraph->vertexLabel("level", vertex));
                preprocessed->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(vertexLevel));
            }
        }

        for (auto clause : instance->hypergraph->hyperedges()) {
            const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(instance->hypergraph->edgeLabel("signs", clause.id()));

            bool deleteClause = false;
            std::vector<vertexNameType> newClause;
            std::vector<bool> newSigns;

            for (unsigned int i = 0; i < clause.size(); i++) {
                bool deleteLiteral = false;
                for (auto unit : unitLiteralsPos) {
                    if (instance->hypergraph->lookupVertex(clause[i]) == unit) {
                        if (edgeSigns[i] == true) {
                            deleteClause = true;
                        } else {
                            deleteLiteral = true;
                        }
                        break;
                    }
                }
                for (auto unit : unitLiteralsNeg) {
                    if (instance->hypergraph->lookupVertex(clause[i]) == unit) {
                        if (edgeSigns[i] == false) {
                            deleteClause = true;
                        } else {
                            deleteLiteral = true;
                        }
                        break;
                    }
                }
                if (deleteClause) {
                    break;
                }
                if (!deleteLiteral) {
                    newClause.push_back(clause[i]);
                    newSigns.push_back(edgeSigns[i]);
                }
            }

            if (!deleteClause) {
                htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(newClause);
                preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(newSigns));
            }
        }

        return preprocessed;
    }

} // namespace preprocessor
