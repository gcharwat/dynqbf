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

#include <cassert>

#include "SplitPreprocessor.h"
#include "../Application.h"

#include <htd/main.hpp>


namespace preprocessor {

    SplitPreprocessor::SplitPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "split", "Split clauses if their length exceeds 100", newDefault) {
    }

    InstancePtr SplitPreprocessor::preprocess(const InstancePtr& instance) const {
        InstancePtr preprocessed(new Instance(app));
        for (const auto q : instance->getQuantifierSequence()) {
            preprocessed->pushBackQuantifier(q);
        }

        if (preprocessed->innermostQuantifier() != NTYPE::EXISTS) {
            preprocessed->pushBackQuantifier(NTYPE::EXISTS);
        }

        // TODO: Improve copy efficiency for vertices
        for (const std::string vertex : instance->hypergraph->vertices()) {
            preprocessed->hypergraph->addVertex(vertex);
            int vertexLevel = htd::accessLabel<int>(instance->hypergraph->vertexLabel("level", vertex));
            preprocessed->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(vertexLevel));
        }

        for (const htd::NamedVertexHyperedge<std::string> edge : instance->hypergraph->hyperedges()) {
            const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(instance->hypergraph->edgeLabel("signs", edge.id()));
            const std::vector<std::string> vertices(edge.begin(), edge.end());
            split(preprocessed, vertices, edgeSigns);
        }

        return preprocessed;
    }
    
    
    void SplitPreprocessor::split(InstancePtr& preprocessed, const std::vector<std::string> &vertices, const std::vector<bool> &edgeSigns) const {
        if (vertices.size() <= 100) {
            htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(vertices);
            preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(edgeSigns));
        } else {
            splitVarIndex++;
            std::stringstream splitVarS;
            splitVarS << "_" << splitVarIndex;
            std::string splitVar = splitVarS.str();
                    
            preprocessed->hypergraph->addVertex(splitVar);
            preprocessed->hypergraph->setVertexLabel("level", splitVar, new htd::Label<int>(preprocessed->quantifierCount()));
            
            std::size_t const halfSize = vertices.size() / 2;
            
            std::vector<std::string> verticesLo(vertices.begin(), vertices.begin() + halfSize);
            verticesLo.push_back(splitVar);
            std::vector<bool> edgeSignsLo(edgeSigns.begin(), edgeSigns.begin() + halfSize);
            edgeSignsLo.push_back(true);
            
            std::vector<std::string> verticesHi(vertices.begin() + halfSize, vertices.end());
            verticesHi.push_back(splitVar);
            std::vector<bool> edgeSignsHi(edgeSigns.begin() + halfSize, edgeSigns.end());
            edgeSignsHi.push_back(false);
            
            split(preprocessed, verticesLo, edgeSignsLo);
            split(preprocessed, verticesHi, edgeSignsHi);
        }
    }
    

} // namespace preprocessor
