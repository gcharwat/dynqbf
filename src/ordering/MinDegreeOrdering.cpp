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

#include "MinDegreeOrdering.h"
#include "../Application.h"
#include <htd/main.hpp>

namespace ordering {

    MinDegreeOrdering::MinDegreeOrdering(Application& app, bool newDefault)
    : Ordering(app, "min-degree", "minimum degree ordering (same as TD min-degree heuristic)", newDefault) {
    }
    
    std::vector<int> MinDegreeOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {
        htd::IOrderingAlgorithm * orderingAlgorithm = new htd::MinDegreeOrderingAlgorithm(app.getHTDManager());
        // TODO: Do not recompute ordering
        htd::IVertexOrdering* ordering = orderingAlgorithm->computeOrdering(instance->hypergraph->internalGraph());
        
        std::vector<int> orderingIndex(instance->hypergraph->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->hypergraph->internalGraph().vertices()) {
            int index = std::find(ordering->sequence().begin(), ordering->sequence().end(), vertexId) - ordering->sequence().begin();
            orderingIndex[vertexId] = index;
        }
        delete ordering;
        delete orderingAlgorithm;
        
        return orderingIndex;
    }
    
}

