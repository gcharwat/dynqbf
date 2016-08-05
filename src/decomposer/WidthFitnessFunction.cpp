
#include "WidthFitnessFunction.h"

namespace decomposer {
        WidthFitnessFunction::WidthFitnessFunction(void)
        {

        }

        WidthFitnessFunction::~WidthFitnessFunction()
        {

        }

        htd::FitnessEvaluation * WidthFitnessFunction::fitness(const htd::IMultiHypergraph & graph, 
                                         const htd::ITreeDecomposition & decomposition) const
        {
            HTD_UNUSED(graph)

            /**
              * Here we specify the fitness evaluation for a given decomposition. 
              * In this case, we select the maximum bag size and the height.
              */
//            return new htd::FitnessEvaluation(2, 
//                                              -(double)(decomposition.maximumBagSize()), 
//                                              -(double)(decomposition.height()));
//            
            return new htd::FitnessEvaluation(1, -(double)(decomposition.maximumBagSize()));
        }

        WidthFitnessFunction * WidthFitnessFunction::clone(void) const
        {
            return new WidthFitnessFunction();
        }
}