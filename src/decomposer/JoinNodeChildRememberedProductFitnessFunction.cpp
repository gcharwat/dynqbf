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

#include "JoinNodeChildRememberedProductFitnessFunction.h"
#include <math.h> 

namespace decomposer {

    JoinNodeChildRememberedProductFitnessFunction::JoinNodeChildRememberedProductFitnessFunction(void) {
    }

    JoinNodeChildRememberedProductFitnessFunction::~JoinNodeChildRememberedProductFitnessFunction() {
    }

    htd::FitnessEvaluation * JoinNodeChildRememberedProductFitnessFunction::fitness(const htd::IMultiHypergraph & graph,
            const htd::ITreeDecomposition & decomposition) const {
        std::vector<htd::vertex_t> joinNodes;

        decomposition.copyJoinNodesTo(joinNodes);

        double joinNodeChildRememberedSum = 0.0;

        for (htd::vertex_t joinNode : joinNodes) {
            double joinNodeChildRememberedProduct = 1.0;
            for (htd::vertex_t childNode : decomposition.children(joinNode)) {
                joinNodeChildRememberedProduct *= decomposition.rememberedVertexCount(joinNode, childNode);
            }
            joinNodeChildRememberedSum += joinNodeChildRememberedProduct;
        }
        return new htd::FitnessEvaluation(1, -joinNodeChildRememberedSum);

    }

    JoinNodeChildRememberedProductFitnessFunction * JoinNodeChildRememberedProductFitnessFunction::clone(void) const {
        return new JoinNodeChildRememberedProductFitnessFunction();
    }
}
