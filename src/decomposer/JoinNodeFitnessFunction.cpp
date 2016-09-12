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

#include "JoinNodeFitnessFunction.h"
#include <math.h> 

// TODO: This will soon be removed!

namespace decomposer {

    JoinNodeFitnessFunction::JoinNodeFitnessFunction(void) {
    }

    JoinNodeFitnessFunction::~JoinNodeFitnessFunction() {
    }

    htd::FitnessEvaluation * JoinNodeFitnessFunction::fitness(const htd::IMultiHypergraph & graph,
            const htd::ITreeDecomposition & decomposition) const {
        HTD_UNUSED(graph)

        std::vector<htd::vertex_t> joinNodes;

        decomposition.copyJoinNodesTo(joinNodes);

        double medianJoinNodeBagSize = 0.0;
        double averageJoinNodeBagSize = 0.0;
        double averageJoinNodeBagSize50 = 0.0;
        double joinNodeChildrenCount = 0.0;
        double averageJoinNodeChildrenCount = 0.0;
        double averageJoinNodeComplexity = 0.0;
        double maximumJoinNodeBagSize = 0.0;

        if (joinNodes.size() > 0) {
            std::vector<double> bagSizes;
            bagSizes.reserve(joinNodes.size());

            for (htd::vertex_t joinNode : joinNodes) {
                double bagSize = decomposition.bagSize(joinNode);

                bagSizes.push_back(bagSize);

                averageJoinNodeBagSize += bagSize;
                joinNodeChildrenCount += decomposition.childCount(joinNode);
                averageJoinNodeComplexity += bagSize * decomposition.childCount(joinNode);

                if (bagSize > maximumJoinNodeBagSize) {
                    maximumJoinNodeBagSize = bagSize;
                }
            }

            std::sort(bagSizes.begin(), bagSizes.end());

            for (unsigned int i = bagSizes.size() / 2; i < bagSizes.size(); i++) {
                averageJoinNodeBagSize50 += bagSizes[i];
            }
            if (bagSizes.size() > 1) {
                averageJoinNodeBagSize50 /= (bagSizes.size()) - bagSizes.size() / 2;
            }

            if (bagSizes.size() % 2 == 0) {
                medianJoinNodeBagSize = (bagSizes[bagSizes.size() / 2 - 1] + bagSizes[bagSizes.size() / 2]) / 2;
            } else {
                medianJoinNodeBagSize = bagSizes[bagSizes.size() / 2];
            }
        }

        averageJoinNodeBagSize /= round(joinNodes.size() / 10);
        averageJoinNodeChildrenCount = joinNodeChildrenCount / joinNodes.size();
        averageJoinNodeComplexity /= joinNodes.size();

        /**
         * Here we specify the fitness evaluation for a given decomposition.
         * In this case, we select the maximum bag size and the height.
         */
        //        return new htd::FitnessEvaluation(4,
        //                -(double) (decomposition.maximumBagSize()),
        //                -medianJoinNodeBagSize,
        //                -(double) (decomposition.joinNodeCount()),
        //                -averageJoinNodeBagSize);
        return new htd::FitnessEvaluation(1,
                //                -(double) (decomposition.maximumBagSize()),
                //                - averageJoinNodeComplexity,
                -(double) (joinNodes.size())
                //                -averageJoinNodeBagSize
                //                -averageJoinNodeBagSize50
                //                -joinNodeChildrenCount
                //                -maximumJoinNodeBagSize
                //                -averageJoinNodeChildrenCount,
                );

    }

    JoinNodeFitnessFunction * JoinNodeFitnessFunction::clone(void) const {
        return new JoinNodeFitnessFunction();
    }
}