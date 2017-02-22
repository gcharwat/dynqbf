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

#include <numeric> 

#include "SingleNodeDecomposer.h"
#include "../Application.h"
#include "../Printer.h"

namespace decomposer {

    SingleNodeDecomposer::SingleNodeDecomposer(Application& app, bool newDefault)
    : Decomposer(app, "single", "Tree decomposition with a single node containing all vertices", newDefault) {
    }

    HTDDecompositionPtr SingleNodeDecomposer::decompose(const InstancePtr& instance) const {

        htd::IMutableTreeDecomposition * decompMutable = app.getHTDManager()->treeDecompositionFactory().createInstance();
        decompMutable->insertRoot();
        std::vector<htd::vertex_t>& bag = decompMutable->mutableBagContent(decompMutable->root());
        const htd::ConstCollection<htd::vertex_t> & vertices = instance->hypergraph->internalGraph().vertices();

        std::copy(vertices.begin(), vertices.end(), std::back_inserter(bag));
        
        htd::FilteredHyperedgeCollection& inducedEdges = decompMutable->mutableInducedHyperedges(decompMutable->root());
        
        std::vector<htd::index_t> indices(instance->hypergraph->internalGraph().edgeCount());
        std::iota(std::begin(indices), std::end(indices), 0);
        
        htd::FilteredHyperedgeCollection originalEdges = instance->hypergraph->internalGraph().hyperedgesAtPositions(indices);
        
        inducedEdges = originalEdges;
        
        HTDDecompositionPtr decomposition(decompMutable);
        
        return decomposition;
    }

} // namespace decomposer
