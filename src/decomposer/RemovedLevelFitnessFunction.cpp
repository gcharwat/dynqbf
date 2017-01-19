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

#include "RemovedLevelFitnessFunction.h"
#include "cuddInt.h"

namespace decomposer {

    RemovedLevelFitnessFunction::RemovedLevelFitnessFunction(Application& app)
    : app(app) {
    }

    RemovedLevelFitnessFunction::~RemovedLevelFitnessFunction() {
    }

    htd::FitnessEvaluation * RemovedLevelFitnessFunction::fitness(const htd::IMultiHypergraph & graph,
            const htd::ITreeDecomposition & decomposition) const {

        double totalCosts = 0;
        removedCountRec(decomposition, decomposition.root(), totalCosts);
        
        return new htd::FitnessEvaluation(1, -totalCosts);
    }
    
    std::vector<unsigned int> RemovedLevelFitnessFunction::removedCountRec(const htd::ITreeDecomposition & decomposition, htd::vertex_t node, double& totalCosts) const {
        std::vector<unsigned int> removedCount(app.getInputInstance()->quantifierCount(), 0);
        // child removals
        for (htd::vertex_t childNode : decomposition.children(node)) {
            std::vector<unsigned int> childRemovedCount = removedCountRec(decomposition, childNode, totalCosts);
            for (unsigned int index = 0; index < childRemovedCount.size(); index++) {
                removedCount.at(index) += childRemovedCount.at(index);
            }
        }
        
        // costs
        double removedWithLowerLevel = 0.0;
        for (htd::vertex_t variable : decomposition.forgottenVertices(node)) {
            unsigned int variableLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->vertexLabel("level", app.getInputInstance()->hypergraph->vertexName(variable)));
            for (unsigned int index = 0; index < variableLevel; index++) {
                removedWithLowerLevel += removedCount.at(index);
            }
        }
        totalCosts += removedWithLowerLevel;
        
        // current removals
        for (htd::vertex_t variable : decomposition.forgottenVertices(node)) {
            unsigned int variableLevel = htd::accessLabel<int>(app.getInputInstance()->hypergraph->vertexLabel("level", app.getInputInstance()->hypergraph->vertexName(variable)));
            removedCount.at(variableLevel - 1) += 1;
        }
        return removedCount;
    }

    RemovedLevelFitnessFunction * RemovedLevelFitnessFunction::clone(void) const {
        return new RemovedLevelFitnessFunction(app);
    }
}