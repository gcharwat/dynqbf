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

#include "LevelOrdering.h"
#include "../Application.h"

namespace ordering {

    LevelOrdering::LevelOrdering(Application& app, bool newDefault)
    : Ordering(app, "level", "ordering based on level (depth) in quantifier sequence", newDefault) {
    }

    std::vector<int> LevelOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {
        const htd::ConstCollection<htd::vertex_t> vertices = instance->hypergraph->internalGraph().vertices();
        std::vector<htd::vertex_t> verticesSorted(vertices.begin(), vertices.end());

        std::sort(verticesSorted.begin(), verticesSorted.end(), [instance] (htd::vertex_t x1, htd::vertex_t x2) -> bool {
            int x1Level = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", x1));
            int x2Level = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", x2));
            if (x1Level < x2Level) {
                return true; // return true if x1 should be ordered before x2 (ie x1 is less than x2)
            } else if (x1Level < x2Level) {
                return x1 < x2; // for same level, keep instance ordering
            }
            return false;
        });

        std::vector<int> orderingIndex(instance->hypergraph->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->hypergraph->internalGraph().vertices()) {
            int index = std::find(verticesSorted.begin(), verticesSorted.end(), vertexId) - verticesSorted.begin();
            orderingIndex[vertexId] = index;
        }

        return orderingIndex;

    }

}

