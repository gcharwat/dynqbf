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

#include <iostream>

#include "Performance.h"
#include "../Application.h"
#include "../Utils.h"

namespace printer {

    Performance::Performance(Application& app, bool newDefault)
    : Printer(app, "performance", "Print performance statistics", newDefault) {
        beginClock = std::clock();
        intermediateBeginClock = std::clock();
    }

    void Performance::inputInstance(const InstancePtr& instance) {
        Printer::inputInstance(instance);
        intermediateEndClock = std::clock();
        double elapsed_secs = double(intermediateEndClock - intermediateBeginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Input (parsing time): " << elapsed_secs << std::endl;
        intermediateBeginClock = std::clock();
        std::cout << "Input (vertices): " << instance->hypergraph->vertexCount() << std::endl;
        std::cout << "Input (edges): " << instance->hypergraph->edgeCount() << std::endl;
        std::cout << "Input (quantifiers): " << instance->quantifierCount() << std::endl;
        std::cout << "Input (first quantifier): " << (instance->quantifier(1) == NTYPE::EXISTS ? "E" : "A") << std::endl;
    }

    void Performance::preprocessedInstance(const InstancePtr& instance) {
        Printer::preprocessedInstance(instance);
        intermediateEndClock = std::clock();
        double elapsed_secs = double(intermediateEndClock - intermediateBeginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Preprocessing (time): " << elapsed_secs << std::endl;
        intermediateBeginClock = std::clock();
        std::cout << "Preprocessing (vertices): " << instance->hypergraph->vertexCount() << std::endl;
        std::cout << "Preprocessing (edges): " << instance->hypergraph->edgeCount() << std::endl;
        std::cout << "Preprocessing (quantifiers): " << instance->quantifierCount() << std::endl;
        std::cout << "Preprocessing (first quantifier): " << (instance->quantifier(1) == NTYPE::EXISTS ? "E" : "A") << std::endl;
    }

    void Performance::decomposerResult(const HTDDecompositionPtr& result) {
        Printer::decomposerResult(result);
        intermediateEndClock = std::clock();
        double elapsed_secs = double(intermediateEndClock - intermediateBeginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Decomposition (decomposing time): " << elapsed_secs << std::endl;
        std::cout << "Decomposition (width): " << (result->maximumBagSize() - 1) << std::endl;

        intermediateBeginClock = std::clock();
    }

    void Performance::vertexOrdering(const std::vector<int>& ordering) {
        Printer::vertexOrdering(ordering);
        intermediateEndClock = std::clock();
        double elapsed_secs = double(intermediateEndClock - intermediateBeginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Ordering (time): " << elapsed_secs << std::endl;
        intermediateBeginClock = std::clock();
    }

    void Performance::afterComputation() {
        intermediateEndClock = std::clock();
        double elapsed_secs = double(intermediateEndClock - intermediateBeginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Solve (time): " << elapsed_secs << std::endl;
        intermediateBeginClock = std::clock();

        double elapsed_total_secs = double(intermediateEndClock - beginClock) / CLOCKS_PER_SEC;
        std::cout << std::fixed << "Total (time): " << elapsed_total_secs << std::endl;
    }

} // namespace printer
