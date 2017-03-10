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

#include "MaxBagOrdering.h"
#include "../Application.h"

#include <algorithm>
#include <limits.h>
//#include <math.h>
//#include <cstddef>
//#include <gmpxx.h>

namespace ordering {

    MaxBagOrdering::MaxBagOrdering(Application& app, bool newDefault)
    : Ordering(app, "max-bag", "vertices together in many bags are close in the ordering (tends to be slow, experimental)", newDefault) {
    }

    std::vector<int> MaxBagOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {

        map = new unsigned int*[instance->hypergraph->vertexCount() + 1];
        for (unsigned int i = 0; i <= instance->hypergraph->vertexCount(); ++i) {
            map[i] = new unsigned int[instance->hypergraph->vertexCount() + 1];
            for (unsigned int j = 0; j <= instance->hypergraph->vertexCount(); ++j) {
                map[i][j] = 0;
            }
        }

        for (const auto node : decomposition->vertices()) {
            for (const auto v1 : decomposition->bagContent(node)) {
                for (const auto v2 : decomposition->bagContent(node)) {
                    map[v1][v2]++;
                }
            }
        }

        std::vector<htd::vertex_t> orderingVertices(instance->hypergraph->internalGraph().vertices().begin(), instance->hypergraph->internalGraph().vertices().end());

        int maxCosts = costs(orderingVertices);

        while (true) {

            //std::cout << "c: " << maxCosts << std::endl;
            //std::cout << "o: " << ordering << std::endl;

            int bestSwapFrom = 0;
            int bestSwapTo = 0;
            int bestCosts = INT_MAX;
            for (unsigned int from = 0; from < orderingVertices.size() - 1; from++) {
                for (unsigned int to = from + 1; to < orderingVertices.size(); to++) {

                    int costsFromBefore = costsVertex(orderingVertices, from);
                    int costsToBefore = costsVertex(orderingVertices, to);

                    std::iter_swap(orderingVertices.begin() + from, orderingVertices.begin() + to);

                    int costsFromAfter = costsVertex(orderingVertices, from);
                    int costsToAfter = costsVertex(orderingVertices, to);

                    int tmpCosts = maxCosts - 2 * costsFromBefore - 2 * costsToBefore + 2 * costsFromAfter + 2 * costsToAfter;

                    if (tmpCosts < bestCosts) {
                        bestSwapFrom = from;
                        bestSwapTo = to;
                        bestCosts = tmpCosts;
                    } else if (tmpCosts == bestCosts) {
                        int fromLevel = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", orderingVertices[from]));
                        int toLevel = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", orderingVertices[to]));
                        if (fromLevel > toLevel) {
                            bestSwapFrom = from;
                            bestSwapTo = to;
                            bestCosts = tmpCosts;
                        }
                    }
                    std::iter_swap(orderingVertices.begin() + from, orderingVertices.begin() + to); // swap back
                }
            }

            if (bestCosts < maxCosts) {
                std::iter_swap(orderingVertices.begin() + bestSwapFrom, orderingVertices.begin() + bestSwapTo);
                maxCosts = bestCosts;
            } else {
                break;
            }
        }

        std::vector<int> orderingIndex(instance->hypergraph->vertexCount() + 1); // "0" vertex is skipped by HTD
        for (const auto& vertexId : instance->hypergraph->internalGraph().vertices()) {
            int index = std::find(orderingVertices.begin(), orderingVertices.end(), vertexId) - orderingVertices.begin();
            orderingIndex[vertexId] = index;
        }

        return orderingIndex;
    }

    int MaxBagOrdering::costs(const std::vector<htd::vertex_t> ordering) const {
        int costs = 0;
        for (unsigned int i = 0; i < ordering.size(); i++) {
            costs += costsVertex(ordering, i);
        }
        return costs;
    }

    int MaxBagOrdering::costsVertex(const std::vector<htd::vertex_t> ordering, const unsigned int vertexPosition) const {
        htd::vertex_t vertex = ordering[vertexPosition];
        unsigned int* vertexLine = map[vertex];

        int costs = 0;
        for (unsigned int i = 0; i < ordering.size(); i++) {
            unsigned int togetherInBags = vertexLine[ordering[i]];
            int distance = 0;
            if (i < vertexPosition) {
                distance = vertexPosition - i - 1;
            } else if (i > vertexPosition) {
                distance = i - vertexPosition - 1;
            }
            costs += distance * togetherInBags;
        }
        return costs;
    }
}

