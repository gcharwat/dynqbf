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

#include <cassert>

#include "CNF3Preprocessor.h"
#include "../Application.h"

#include <htd/main.hpp>


namespace preprocessor {

    CNF3Preprocessor::CNF3Preprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "3cnf", "Convert instance to 3-CNF", newDefault) {
    }

    InstancePtr CNF3Preprocessor::preprocess(const InstancePtr& instance) const {
        InstancePtr preprocessed(new Instance(app));
        for (const auto q : instance->getQuantifierSequence()) {
            preprocessed->pushBackQuantifier(q);

        }

        // TODO: Improve copy efficiency for vertices
        for (vertexNameType vertex : instance->hypergraph->vertices()) {
            preprocessed->hypergraph->addVertex(vertex);
            int vertexLevel = htd::accessLabel<int>(instance->hypergraph->vertexLabel("level", vertex));
            preprocessed->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(vertexLevel));
        }

        vertexNameType nextVertex = 0;
        for (const auto vertex : preprocessed->hypergraph->vertices()) {
            if (nextVertex < vertex) {
                nextVertex = vertex;
            }
        }

        for (const htd::NamedVertexHyperedge<vertexNameType> edge : instance->hypergraph->hyperedges()) {
            const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(instance->hypergraph->edgeLabel("signs", edge.id()));
            if (edge.size() < 4) {
                std::vector<vertexNameType> vertices(edge.begin(), edge.end());
                htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(vertices);
                preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(edgeSigns));
            } else {
                if (preprocessed->innermostQuantifier() != NTYPE::EXISTS) {
                    preprocessed->pushBackQuantifier(NTYPE::EXISTS);
                }

                std::vector<vertexNameType> vertices;
                vertices.push_back(edge[0]);
                vertices.push_back(edge[1]);
                nextVertex++;
                vertices.push_back(nextVertex);

                preprocessed->hypergraph->addVertex(nextVertex);
                preprocessed->hypergraph->setVertexLabel("level", nextVertex, new htd::Label<int>(preprocessed->quantifierCount()));

                std::vector<bool> signs;
                signs.push_back(edgeSigns[0]);
                signs.push_back(edgeSigns[1]);
                signs.push_back(true);

                htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(vertices);
                preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(signs));

                for (unsigned int i = 2; i < edge.size() - 2; i++) {

                    std::vector<vertexNameType> verticesIntermediate;
                    verticesIntermediate.push_back(nextVertex);
                    verticesIntermediate.push_back(edge[i]);
                    nextVertex++;
                    verticesIntermediate.push_back(nextVertex);

                    preprocessed->hypergraph->addVertex(nextVertex);
                    preprocessed->hypergraph->setVertexLabel("level", nextVertex, new htd::Label<int>(preprocessed->quantifierCount()));

                    std::vector<bool> signsIntermediate;
                    signsIntermediate.push_back(false);
                    signsIntermediate.push_back(edgeSigns[i]);
                    signsIntermediate.push_back(true);

                    htd::id_t newEdgeIdIntermediate = preprocessed->hypergraph->addEdge(verticesIntermediate);
                    preprocessed->hypergraph->setEdgeLabel("signs", newEdgeIdIntermediate, new htd::Label < std::vector<bool>>(signsIntermediate));

                }

                std::vector<vertexNameType> verticesEnd;

                verticesEnd.push_back(nextVertex);
                verticesEnd.push_back(edge[edge.size() - 2]);
                verticesEnd.push_back(edge[edge.size() - 1]);

                std::vector<bool> signsEnd;
                signsEnd.push_back(false);
                signsEnd.push_back(edgeSigns[edge.size() - 2]);
                signsEnd.push_back(edgeSigns[edge.size() - 1]);

                htd::id_t newEdgeIdEnd = preprocessed->hypergraph->addEdge(verticesEnd);
                preprocessed->hypergraph->setEdgeLabel("signs", newEdgeIdEnd, new htd::Label < std::vector<bool>>(signsEnd));

            }
        }

        return preprocessed;
    }

} // namespace preprocessor
