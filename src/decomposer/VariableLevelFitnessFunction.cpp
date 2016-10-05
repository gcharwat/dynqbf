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

#include "VariableLevelFitnessFunction.h"

namespace decomposer {

    VariableLevelFitnessFunction::VariableLevelFitnessFunction(Application& app)
    : app(app) {
    }

    VariableLevelFitnessFunction::~VariableLevelFitnessFunction() {
    }

    htd::FitnessEvaluation * VariableLevelFitnessFunction::fitness(const htd::IMultiHypergraph & graph,
            const htd::ITreeDecomposition & decomposition) const {

        //assert(app.getInputInstance()->hypergraph->internalGraph() == graph);
        const unsigned int quantifierCount = app.getInputInstance()->quantifierCount();
        
        double positionDistanceSum = 0.0;

        for (htd::vertex_t node : decomposition.vertices()) {
            std::size_t depth = decomposition.depth(node);
            std::size_t height = decomposition.height(node);
            double relativeNodePosition = depth / (double) (depth + height);
            
            for (htd::vertex_t variable : decomposition.forgottenVertices(node)) {
                unsigned int variableLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->vertexLabel("level", app.getInputInstance()->hypergraph->vertexName(variable)));
                double relativeVariablePosition = (variableLevel - 1) / (double) (quantifierCount - 1);
                
                double relativeDistance = relativeNodePosition - relativeVariablePosition;
                if (relativeDistance < 0) {
                    relativeDistance *= -1;
                }
                positionDistanceSum += relativeDistance;
            }
        }
        
        
        //        for (htd::vertex_t node : decomposition.vertices()) {
        //            double relativeVertexPosition = decomposition->depth(node) / (double) (decomposition->depth(node) + decomposition->height(node));
        //            for (htd::vertex_t vertex : decomposition->bagContent(node)) {
        //                double maxLevel = 2; // TODO get from instance
        ////                unsigned int variableLevel = htd::accessLabel<int>(graph->->vertexLabel("level", vertex));
        ////                double relativeVariableLevel = ;
        //            }
        //        }
        ////        
        //        std::vector<htd::vertex_t> joinNodes;
        //
        //        decomposition.copyJoinNodesTo(joinNodes);
        //
        //        double joinNodeChildBagSum = 0.0;
        //        
        //        for (htd::vertex_t joinNode : joinNodes) {
        //            for (htd::vertex_t childNode : decomposition.children(joinNode)) {
        //                joinNodeChildBagSum += decomposition.bagSize(childNode);
        //            }
        //        }
        //            return new htd::FitnessEvaluation(1, -joinNodeChildBagSum);
        return new htd::FitnessEvaluation(1, -positionDistanceSum);
    }

    VariableLevelFitnessFunction * VariableLevelFitnessFunction::clone(void) const {
        return new VariableLevelFitnessFunction(app);
    }
}