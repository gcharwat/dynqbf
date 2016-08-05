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
class WidthFitnessFunction : public htd::ITreeDecompositionFitnessFunction
{
    public:
        WidthFitnessFunction(void);
        ~WidthFitnessFunction();

        htd::FitnessEvaluation * fitness(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const;

        WidthFitnessFunction * clone(void) const;
};
}
