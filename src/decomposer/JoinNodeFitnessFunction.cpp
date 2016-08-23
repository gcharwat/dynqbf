
#include "JoinNodeFitnessFunction.h"
#include <math.h> 

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
                averageJoinNodeChildrenCount += decomposition.childCount(joinNode);
                averageJoinNodeComplexity += bagSize * decomposition.childCount(joinNode);
                
                if (bagSize > maximumJoinNodeBagSize) {
                    maximumJoinNodeBagSize = bagSize;
                }
            }

            
            std::sort(bagSizes.begin(), bagSizes.end());

            if (bagSizes.size() % 2 == 0) {
                medianJoinNodeBagSize = (bagSizes[bagSizes.size() / 2 - 1] + bagSizes[bagSizes.size() / 2]) / 2;
            } else {
                medianJoinNodeBagSize = bagSizes[bagSizes.size() / 2];
            }
        }

            averageJoinNodeBagSize /= round(joinNodes.size()/10);
            averageJoinNodeChildrenCount /= joinNodes.size();
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
//                -(double) (joinNodes.size())
//                -averageJoinNodeBagSize
                -maximumJoinNodeBagSize
//                -averageJoinNodeChildrenCount,
                );

    }

    JoinNodeFitnessFunction * JoinNodeFitnessFunction::clone(void) const {
        return new JoinNodeFitnessFunction();
    }
}