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

#include <algorithm>

#include "LexicographicalOrdering.h"
#include "../Application.h"

namespace ordering {

    LexicographicalOrdering::LexicographicalOrdering(Application& app, bool newDefault)
    : Ordering(app, "lexicographic", "lexicographic ordering", newDefault) {
    }

    std::vector<int> LexicographicalOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {
        std::vector<vertexNameType> vertices;
        for (const auto vertex : instance->hypergraph->vertices()) {
            vertices.push_back(vertex);
        }
        std::sort(vertices.begin(), vertices.end());

        std::vector<int> orderingIndex(instance->hypergraph->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->hypergraph->internalGraph().vertices()) {
            vertexNameType vertex = instance->hypergraph->vertexName(vertexId);
            int index = std::find(vertices.begin(), vertices.end(), vertex) - vertices.begin();
            orderingIndex[vertexId] = index;
        }

        return orderingIndex;

    }

}

