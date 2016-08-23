/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   WidthFitnessFunction.h
 * Author: gcharwat
 *
 * Created on August 4, 2016, 10:51 AM
 */


#include <htd/main.hpp>

namespace decomposer {
class JoinNodeFitnessFunction : public htd::ITreeDecompositionFitnessFunction
{
    public:
        JoinNodeFitnessFunction(void);
        ~JoinNodeFitnessFunction();

        htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const;

        JoinNodeFitnessFunction * clone(void) const;
};
}
