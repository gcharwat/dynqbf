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

#include "MaxClauseOrdering.h"
#include "../Application.h"

#include <algorithm>
#include <limits.h>
#include <math.h>

namespace ordering {

    MaxClauseOrdering::MaxClauseOrdering(Application& app, bool newDefault)
    : Ordering(app, "max-clause", "vertices together in many clauses are close in the ordering (tends to be slow, experimental)", newDefault) {
    }

    std::vector<int> MaxClauseOrdering::computeVertexOrder(const InstancePtr& instance, const HTDDecompositionPtr& decomposition) const {

        map = new unsigned int*[instance->hypergraph->vertexCount() + 1];
        for (unsigned int i = 0; i <= instance->hypergraph->vertexCount(); ++i) {
            map[i] = new unsigned int[instance->hypergraph->vertexCount() + 1];
            for (unsigned int j = 0; j <= instance->hypergraph->vertexCount(); ++j) {
                map[i][j] = 0;
            }
        }

        for (const auto edge : instance->hypergraph->internalGraph().hyperedges()) {
            for (const auto v1 : edge) {
                for (const auto v2 : edge) {
                    map[v1][v2]++;
                }
            }
        }

        std::vector<htd::vertex_t> orderingVertices;
        for (const auto v : instance->hypergraph->internalGraph().vertices()) {

            int bestPos = 0;
            int bestCosts = INT_MAX;

            for (unsigned int pos = 0; pos <= orderingVertices.size(); pos++) {
                int tmpCosts = costsNewVertex(orderingVertices, pos, v);
                if (tmpCosts <= bestCosts) {
                    bestPos = pos;
                    bestCosts = tmpCosts;
                }
            }
            orderingVertices.insert(orderingVertices.begin() + bestPos, v);
        }

        //std::cout << "before costs: " << std::endl;

        int maxCosts = costs(orderingVertices);
        int maxItations = INT_MAX;

        while (maxItations > 0) {
            maxItations--;

            int bestSwapFrom = 0;
            int bestSwapTo = 0;
            int bestCosts = INT_MAX;

            int* costsBefore = new int[orderingVertices.size()];
            for (unsigned int index = 0; index < orderingVertices.size(); index++) {
                costsBefore[index] = costsVertex(orderingVertices, index);
            }

            for (unsigned int from = 0; from < orderingVertices.size() - 1; from++) {

                int costsFromBefore = costsBefore[from];

                for (unsigned int to = from + 1; to < orderingVertices.size(); to++) {

                    int costsToBefore = costsBefore[to];

                    std::swap(orderingVertices[from], orderingVertices[to]);

                    int costsFromAfter = costsVertex(orderingVertices, from);
                    int costsToAfter = costsVertex(orderingVertices, to);

                    int tmpCosts = maxCosts - 2 * costsFromBefore - 2 * costsToBefore + 2 * costsFromAfter + 2 * costsToAfter;

                    if (tmpCosts < bestCosts || (tmpCosts == bestCosts && orderingVertices[to] < orderingVertices[from])) {
                        bestSwapFrom = from;
                        bestSwapTo = to;
                        bestCosts = tmpCosts;
                    }
                    std::swap(orderingVertices[from], orderingVertices[to]); // swap back
                }
            }

            if (bestCosts < maxCosts) {
                std::swap(orderingVertices[bestSwapFrom], orderingVertices[bestSwapTo]);
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

    int MaxClauseOrdering::costs(const std::vector<htd::vertex_t> ordering) const {
        int costs = 0;
        for (unsigned int i = 0; i < ordering.size(); i++) {
            costs += costsVertex(ordering, i);
        }
        return costs;
    }

    int MaxClauseOrdering::costsVertex(const std::vector<htd::vertex_t> ordering, const unsigned int vertexPosition) const {
        htd::vertex_t vertex = ordering[vertexPosition];
        unsigned int* vertexLine = map[vertex];

        int costs = 0;
        for (unsigned int i = 0; i < ordering.size(); i++) {
            unsigned int togetherInClauses = vertexLine[ordering[i]];
            int distance = 0;
            if (i < vertexPosition) {
                distance = vertexPosition - i - 1;
            } else if (i > vertexPosition) {
                distance = i - vertexPosition - 1;
            }
            costs += distance * togetherInClauses;
        }
        return costs;
    }

    int MaxClauseOrdering::costsNewVertex(const std::vector<htd::vertex_t>& ordering, const unsigned int newVertexPosition, const htd::vertex_t newVertex) const {
        unsigned int* vertexLine = map[newVertex];

        int costs = 0;
        unsigned int i = 0;
        for (const auto v : ordering) {
            int togetherInClauses = vertexLine[v];
            int distance = 0;
            if (i < newVertexPosition) {
                distance = newVertexPosition - i - 1;
            } else if (i > newVertexPosition) {
                distance = i - newVertexPosition;
            }
            costs += distance * togetherInClauses;
            i++;
        }
        return costs;
    }
}

