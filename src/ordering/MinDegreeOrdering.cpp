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

#include "MinDegreeOrdering.h"
#include "../Application.h"
#include <htd/main.hpp>

namespace ordering {

    MinDegreeOrdering::MinDegreeOrdering(Application& app, bool newDefault)
    : Ordering(app, "min-degree", "minimum degree ordering (same as TD min-degree heuristic)", newDefault) {
    }
    
    std::vector<int> MinDegreeOrdering::computeVertexOrder(const HTDHypergraphPtr& instance, const HTDDecompositionPtr& decomposition) const {
        htd::IOrderingAlgorithm * orderingAlgorithm = new htd::MinDegreeOrderingAlgorithm(app.getHTDManager());
        std::vector<htd::vertex_t> orderingVertices;
        // TODO: Do not recompute ordering
        orderingAlgorithm->writeOrderingTo(instance->internalGraph(), orderingVertices);
        delete orderingAlgorithm;
        
        std::vector<int> orderingIndex(instance->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->internalGraph().vertices()) {
            int index = std::find(orderingVertices.begin(), orderingVertices.end(), vertexId) - orderingVertices.begin();
            orderingIndex[vertexId] = index;
        }
        
        return orderingIndex;
    }
    
}

