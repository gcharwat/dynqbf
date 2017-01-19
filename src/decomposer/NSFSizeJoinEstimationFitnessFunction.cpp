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

#include "NSFSizeJoinEstimationFitnessFunction.h"
#include <math.h> 

namespace decomposer {

    NSFSizeJoinEstimationFitnessFunction::NSFSizeJoinEstimationFitnessFunction(void) {
    }

    NSFSizeJoinEstimationFitnessFunction::~NSFSizeJoinEstimationFitnessFunction() {
    }

    
    htd::FitnessEvaluation* NSFSizeJoinEstimationFitnessFunction::fitness(const htd::IMultiHypergraph& graph,
            const htd::ITreeDecomposition& decomposition) const {
        
        double nsfSizeEstimation = 0.0;
        removedCount(decomposition, decomposition.root(), nsfSizeEstimation);
        
        return new htd::FitnessEvaluation(1, -nsfSizeEstimation);

    }
    
    double NSFSizeJoinEstimationFitnessFunction::removedCount(const htd::ITreeDecomposition& decomposition, htd::vertex_t node, double& fitness) const {
        double value = 0.0;
        if (!decomposition.isLeaf(node)) {
            for (htd::vertex_t child : decomposition.children(node)) {
                value += removedCount(decomposition, child, fitness);
            }
            value += decomposition.forgottenVertexCount(node);
        }
        if (decomposition.isJoinNode(node)) {
            fitness += value;
        }
        return value;
    }

    NSFSizeJoinEstimationFitnessFunction * NSFSizeJoinEstimationFitnessFunction::clone(void) const {
        return new NSFSizeJoinEstimationFitnessFunction();
    }
}