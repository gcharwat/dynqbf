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

#include "JoinNodeChildBagProductFitnessFunction.h"
#include <math.h> 

namespace decomposer {

    JoinNodeChildBagProductFitnessFunction::JoinNodeChildBagProductFitnessFunction(void) {
    }

    JoinNodeChildBagProductFitnessFunction::~JoinNodeChildBagProductFitnessFunction() {
    }

    htd::FitnessEvaluation * JoinNodeChildBagProductFitnessFunction::fitness(const htd::IMultiHypergraph & graph,
            const htd::ITreeDecomposition & decomposition) const {
        
        double instanceSize = graph.vertexCount();
        
        std::vector<htd::vertex_t> joinNodes;

        decomposition.copyJoinNodesTo(joinNodes);

        double joinNodeChildBagSum = 0.0;

        for (htd::vertex_t joinNode : joinNodes) {
            double joinNodeChildBagProduct = 1.0;
            for (htd::vertex_t childNode : decomposition.children(joinNode)) {
                joinNodeChildBagProduct *= (decomposition.bagSize(childNode) / instanceSize);
            }
            joinNodeChildBagSum += joinNodeChildBagProduct;
        }
        return new htd::FitnessEvaluation(1, -joinNodeChildBagSum);

    }

    JoinNodeChildBagProductFitnessFunction * JoinNodeChildBagProductFitnessFunction::clone(void) const {
        return new JoinNodeChildBagProductFitnessFunction();
    }
}