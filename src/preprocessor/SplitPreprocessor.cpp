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

    const std::string SplitPreprocessor::OPTION_SECTION = "Splitting (-r split)";

    SplitPreprocessor::SplitPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "split", "Split large clauses", newDefault)
    , optSplitSize("split-size", "s", "Split clauses if their length exceeds <s>", 30)
    , optSplitStrategy("split-strategy", "s", "Split clauses following strategy <s>") {

        optSplitSize.addCondition(selected);
        app.getOptionHandler().addOption(optSplitSize, OPTION_SECTION);

        optSplitStrategy.addCondition(selected);
        optSplitStrategy.addChoice("instance", "variables are not reordered before splitting", true);
        optSplitStrategy.addChoice("level", "variables are ordered based on their quantification level before splitting");
        optSplitStrategy.addChoice("id", "variables are ordered based on their first occurrence in the instance");
        app.getOptionHandler().addOption(optSplitStrategy, OPTION_SECTION);

    }

    InstancePtr SplitPreprocessor::preprocess(const InstancePtr& instance) const {
        InstancePtr preprocessed(new Instance(app));
        for (const auto q : instance->getQuantifierSequence()) {
            preprocessed->pushBackQuantifier(q);
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

            if (vertices.size() <= (unsigned int) optSplitSize.getValue()) {
                htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(vertices);
                preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(edgeSigns));
            } else {
                if (preprocessed->innermostQuantifier() != NTYPE::EXISTS) {
                    preprocessed->pushBackQuantifier(NTYPE::EXISTS);
                }

                std::vector < std::pair < std::string, bool>> combinedEdge;
                combinedEdge.reserve(vertices.size());
                std::transform(vertices.begin(), vertices.end(), edgeSigns.begin(), std::back_inserter(combinedEdge),
                        [](std::string v, bool s) {
                            return std::make_pair(v, s);
                        });

                if (optSplitStrategy.getValue() == "level") {
                    std::sort(combinedEdge.begin(), combinedEdge.end(), [preprocessed] (std::pair < std::string, bool> v1, std::pair < std::string, bool> v2) -> bool {
                        unsigned int v1Level = htd::accessLabel<int>(preprocessed->hypergraph->vertexLabel("level", v1.first));
                        unsigned int v2Level = htd::accessLabel<int>(preprocessed->hypergraph->vertexLabel("level", v2.first));
                        return (v1Level < v2Level);
                    });
                } else if (optSplitStrategy.getValue() == "id") {
                    std::sort(combinedEdge.begin(), combinedEdge.end(), [preprocessed] (std::pair < std::string, bool> v1, std::pair < std::string, bool> v2) -> bool {
                        htd::vertex_t v1id = preprocessed->hypergraph->lookupVertex(v1.first);
                        htd::vertex_t v2id = preprocessed->hypergraph->lookupVertex(v2.first);
                        return (v1id < v2id);
                    });
                } else { //  "instance"
                    // do nothing
                }
                split(preprocessed, combinedEdge);
            }
        }

        return preprocessed;
    }

    void SplitPreprocessor::split(InstancePtr& preprocessed, const std::vector < std::pair < std::string, bool>> &combinedEdge) const {
        if (combinedEdge.size() <= (unsigned int) optSplitSize.getValue()) {
            std::vector<std::string> vertices;
            std::transform(combinedEdge.begin(), combinedEdge.end(), std::back_inserter(vertices), [](const std::pair < std::string, bool>& v) {
                return v.first; });
            std::vector<bool> edgeSigns;
            std::transform(combinedEdge.begin(), combinedEdge.end(), std::back_inserter(edgeSigns), [](const std::pair < std::string, bool>& v) {
                return v.second; });
            htd::id_t newEdgeId = preprocessed->hypergraph->addEdge(vertices);
            preprocessed->hypergraph->setEdgeLabel("signs", newEdgeId, new htd::Label < std::vector<bool>>(edgeSigns));
        } else {
            splitVarIndex++;
            std::stringstream splitVarS;
            splitVarS << "_" << splitVarIndex;
            std::string splitVar = splitVarS.str();

            preprocessed->hypergraph->addVertex(splitVar);
            preprocessed->hypergraph->setVertexLabel("level", splitVar, new htd::Label<int>(preprocessed->quantifierCount()));

            std::size_t const halfSize = combinedEdge.size() / 2;

            std::vector < std::pair < std::string, bool>> combinedEdgeLo(combinedEdge.begin(), combinedEdge.begin() + halfSize);
            combinedEdgeLo.push_back(std::make_pair(splitVar, true));
            std::vector < std::pair < std::string, bool>> combinedEdgeHi(combinedEdge.begin() + halfSize, combinedEdge.end());
            combinedEdgeHi.push_back(std::make_pair(splitVar, false));

            split(preprocessed, combinedEdgeLo);
            split(preprocessed, combinedEdgeHi);
        }
    }


} // namespace preprocessor
