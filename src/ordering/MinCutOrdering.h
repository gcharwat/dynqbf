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

#pragma once

#include "../Ordering.h"

namespace ordering {

    class MinCutOrdering : public Ordering {
    public:
        MinCutOrdering(Application& app, bool newDefault = false);

        virtual std::vector<int> computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const override;

    private:
        static const std::string OPTION_SECTION;

        std::vector<htd::vertex_t> minCutRec(const std::vector<htd::vertex_t>& vertices, const std::vector<htd::Hyperedge>& localEdges, const std::vector<htd::Hyperedge>& edgesCommonOld, const bool leftLeaf) const;
        bool edgeContained(const std::vector<htd::vertex_t>& vertices, const htd::Hyperedge& edge) const;
        bool edgeContainedAdditional(const std::vector<htd::vertex_t>& vertices, const htd::vertex_t additionalVertex, const htd::Hyperedge& edge) const;
        bool edgeContainedRemoved(const std::vector<htd::vertex_t>& vertices, const htd::vertex_t removedVertex, const htd::Hyperedge& edge) const;
    };

}