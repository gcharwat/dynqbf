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

#include "Dummy.h"
#include "../Application.h"
#include "../Printer.h"

namespace decomposer {

    Dummy::Dummy(Application& app, bool newDefault)
    : Decomposer(app, "dummy", "Do not decompose", newDefault) {
    }

    HTDDecompositionPtr Dummy::decompose(const HTDHypergraphPtr& instance) const {

        htd::IMutableTreeDecomposition * decompMutable = app.getHTDManager()->treeDecompositionFactory().getTreeDecomposition();
        decompMutable->insertRoot();
        std::vector<htd::vertex_t> & bag = decompMutable->bagContent(decompMutable->root());
        const htd::ConstCollection<htd::vertex_t> & vertices = instance->internalGraph().vertices();

        std::copy(vertices.begin(), vertices.end(), std::back_inserter(bag));

        HTDDecompositionPtr decomposition(decompMutable);
        return decomposition;
    }

} // namespace decomposer
