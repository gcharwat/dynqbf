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

#include "MinCutOrdering.h"
#include "../Application.h"

#include <algorithm>
#include <limits.h>

namespace ordering {

    MinCutOrdering::MinCutOrdering(Application& app, bool newDefault)
    : Ordering(app, "min-cut", "min cut ordering (tends to be slow, experimental)", newDefault) {
    }

    std::vector<int> MinCutOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {

        std::vector<htd::vertex_t> vertices(instance->hypergraph->internalGraph().vertices().begin(), instance->hypergraph->internalGraph().vertices().end());
        std::vector<htd::Hyperedge> localEdges(instance->hypergraph->internalGraph().hyperedges().begin(), instance->hypergraph->internalGraph().hyperedges().end());
        std::vector<htd::Hyperedge> commonEdges;
        std::vector<htd::vertex_t> orderingVertices = minCutRec(vertices, localEdges, commonEdges, true);

        std::vector<int> orderingIndex(instance->hypergraph->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->hypergraph->internalGraph().vertices()) {
            int index = std::find(orderingVertices.begin(), orderingVertices.end(), vertexId) - orderingVertices.begin();
            orderingIndex[vertexId] = index;
        }

        return orderingIndex;
    }

    std::vector<htd::vertex_t> MinCutOrdering::minCutRec(const std::vector<htd::vertex_t>& vertices, const std::vector<htd::Hyperedge>& localEdges, const std::vector<htd::Hyperedge>& edgesCommonOld, const bool leftLeaf) const {
        if (vertices.size() == 0) {
            std::vector<htd::vertex_t> ordering;
            return ordering;
        } else if (vertices.size() == 1) {
            std::vector<htd::vertex_t> ordering;
            ordering.insert(ordering.begin(), vertices.begin(), vertices.end());
            return ordering;
        }

        float ratio = 0.4f;
        int maxSearch = vertices.size();

        std::vector<htd::vertex_t> verticesLeft; 
        std::vector<htd::vertex_t> verticesRight;

        std::vector<htd::Hyperedge> edgesLeft;
        std::vector<htd::Hyperedge> edgesRight;
        std::vector<htd::Hyperedge> edgesCommon;

        int costsSoFar = 0;
        for (auto edge : localEdges) {
            bool inLeft = false;
            bool inRight = false;

            auto i = std::begin(edge);
            while (i != std::end(edge)) {

                if (std::find(vertices.begin(), vertices.end(), *i) == vertices.end()) {
                    edge.erase(*i);
                    continue;
                }

                if (verticesLeft.size() < vertices.size() / 2) {
                    if (std::find(verticesLeft.begin(), verticesLeft.end(), *i) == verticesLeft.end()) {
                        verticesLeft.push_back(*i);
                        inLeft = true;
                    }
                } else {

                    if (std::find(verticesLeft.begin(), verticesLeft.end(), *i) != verticesLeft.end()) {
                        inLeft = true;
                    } else if (std::find(verticesRight.begin(), verticesRight.end(), *i) != verticesRight.end()) {
                        inRight = true;
                    } else {
                        verticesRight.push_back(*i);
                        inRight = true;
                    }
                }
                i++;
            }
            if (!edge.empty()) {
                if (inLeft && inRight) {
                    edgesCommon.push_back(edge);
                    costsSoFar++;
                } else if (inLeft) {
                    edgesLeft.push_back(edge);
                } else if (inRight) {
                    edgesRight.push_back(edge);
                }
            }
        }

        for (const auto v : vertices) {
            if (std::find(verticesLeft.begin(), verticesLeft.end(), v) == verticesLeft.end()) {
                if (std::find(verticesRight.begin(), verticesRight.end(), v) == verticesRight.end()) {
                    if (verticesLeft.size() < verticesRight.size()) {
                        verticesLeft.insert(verticesLeft.begin(), v);
                    } else {
                        verticesRight.push_back(v);
                    }
                }
            }
        }

        for (const auto edge : edgesCommonOld) {
            if (leftLeaf) {
                if (edgeContained(verticesLeft, edge)) {
                    costsSoFar++;
                }
            } else {
                if (edgeContained(verticesRight, edge)) {
                    costsSoFar++;
                }
            }
        }

        while (maxSearch > 0) {
            maxSearch--;

            int minCostsLeft = INT_MAX;
            int minCostsRight = INT_MAX;
            htd::vertex_t removeLeft = htd::Vertex::UNKNOWN;
            htd::vertex_t removeRight = htd::Vertex::UNKNOWN;

            // guess vertex to be moved to right
            for (const auto vertex : verticesLeft) {
                // adapt costs
                int localCosts = costsSoFar;

                for (const auto edge : edgesLeft) {
                    bool inLeft = edgeContainedRemoved(verticesLeft, vertex, edge);
                    bool inRight = edgeContainedAdditional(verticesRight, vertex, edge);
                    if (inLeft && inRight) {
                        localCosts++;
                    }
                }
                for (const auto edge : edgesCommon) {
                    bool inLeft = edgeContainedRemoved(verticesLeft, vertex, edge);
                    if (!inLeft) {
                        localCosts--;
                    }
                }
                for (const auto edge : edgesCommonOld) {
                    if (leftLeaf) {
                        if (!edgeContainedRemoved(verticesLeft, vertex, edge)) {
                            localCosts--;
                        }
                    }
                }
                if (localCosts < minCostsLeft) {
                    minCostsLeft = localCosts;
                    removeLeft = vertex;
                }
            }
            
            for (const auto vertex : verticesRight) {
                // adapt costs
                int localCosts = costsSoFar;

                for (const auto edge : edgesRight) {
                    bool inRight = edgeContainedRemoved(verticesRight, vertex, edge);
                    bool inLeft = edgeContainedAdditional(verticesLeft, vertex, edge);
                    if (inLeft && inRight) {
                        localCosts++;
                    }
                }
                for (const auto edge : edgesCommon) {
                    bool inRight = edgeContainedRemoved(verticesRight, vertex, edge);
                    if (!inRight) {
                        localCosts--;
                    }
                }
                for (const auto edge : edgesCommonOld) {
                    if (!leftLeaf) {
                        if (!edgeContainedRemoved(verticesRight, vertex, edge)) {
                            localCosts--;
                        }
                    }
                }
                if (localCosts < minCostsRight) {
                    minCostsRight = localCosts;
                    removeRight = vertex;
                }
            }

            if (((float) verticesLeft.size() - 1) / ((float) vertices.size()) > ratio && minCostsLeft <= minCostsRight && minCostsLeft < costsSoFar) {
                // move from left to right
                costsSoFar = minCostsLeft;
                verticesLeft.erase(std::remove(verticesLeft.begin(), verticesLeft.end(), removeLeft), verticesLeft.end());
                verticesRight.push_back(removeLeft);
            } else if (((float) verticesRight.size() - 1) / ((float) vertices.size()) > ratio && minCostsRight < minCostsLeft && minCostsRight < costsSoFar) {
                // move from right to left
                costsSoFar = minCostsRight;
                verticesRight.erase(std::remove(verticesRight.begin(), verticesRight.end(), removeRight), verticesRight.end());
                verticesLeft.push_back(removeRight);
            } else if ((((float) verticesRight.size()) / ((float) vertices.size())) < ratio) {
                // move from left to right
                costsSoFar = minCostsLeft;
                verticesLeft.erase(std::remove(verticesLeft.begin(), verticesLeft.end(), removeLeft), verticesLeft.end());
                verticesRight.push_back(removeLeft);
            } else if ((((float) verticesLeft.size()) / ((float) vertices.size())) < ratio) {
                // move from right to left
                costsSoFar = minCostsRight;
                verticesRight.erase(std::remove(verticesRight.begin(), verticesRight.end(), removeRight), verticesRight.end());
                verticesLeft.push_back(removeRight);
            } else {
                break;
            }

            edgesLeft.clear();
            edgesRight.clear();
            edgesCommon.clear();
            for (const auto edge : localEdges) {
                bool inLeft = edgeContained(verticesLeft, edge);
                bool inRight = edgeContained(verticesRight, edge);
                if (inLeft && inRight) {
                    edgesCommon.push_back(edge);
                } else if (inLeft) {
                    edgesLeft.push_back(edge);
                } else if (inRight) {
                    edgesRight.push_back(edge);
                }
            }
        }

        // TODO old common edges handling
        std::vector<htd::vertex_t> orderingLeft = minCutRec(verticesLeft, edgesLeft, edgesCommon, true);
        std::vector<htd::vertex_t> orderingRight = minCutRec(verticesRight, edgesRight, edgesCommon, false);

        orderingLeft.insert(orderingLeft.end(), orderingRight.begin(), orderingRight.end());
        return orderingLeft;
    }

    bool MinCutOrdering::edgeContained(const std::vector<htd::vertex_t>& vertices, const htd::Hyperedge& edge) const {
        for (const auto edgeVertex : edge) {
            if (std::find(vertices.begin(), vertices.end(), edgeVertex) != vertices.end()) {
                return true;
            }
        }
        return false;
    }

    bool MinCutOrdering::edgeContainedAdditional(const std::vector<htd::vertex_t>& vertices, const htd::vertex_t additionalVertex, const htd::Hyperedge& edge) const {
        if (edgeContained(vertices, edge)) {
            return true;
        }
        return std::find(edge.begin(), edge.end(), additionalVertex) != edge.end();
    }

    bool MinCutOrdering::edgeContainedRemoved(const std::vector<htd::vertex_t>& vertices, const htd::vertex_t removedVertex, const htd::Hyperedge& edge) const {
        for (const auto edgeVertex : edge) {
            if (edgeVertex == removedVertex) {
                continue;
            }
            if (std::find(vertices.begin(), vertices.end(), edgeVertex) != vertices.end()) {
                return true;
            }
        }
        return false;
    }

}

