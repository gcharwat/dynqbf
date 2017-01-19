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

#include "Progress.h"
#include "../Application.h"

namespace printer {

    Progress::Progress(Application& app, bool newDefault)
    : Printer(app, "progress", "Print progress", newDefault) {
    }

    void Progress::inputInstance(const InstancePtr & instance) {
        Printer::inputInstance(instance);
        std::cout << "\rParsing done." << std::flush;
        std::cout << std::endl;
    }
    
    void Progress::preprocessedInstance(const InstancePtr& instance) {
        Printer::preprocessedInstance(instance);
        std::cout << "\rPreprocessing done." << std::flush;
        std::cout << std::endl;
    }

    void Progress::decomposerResult(const HTDDecompositionPtr & result) {
        Printer::decomposerResult(result);
        std::cout << "\rDecomposing done." << std::flush;
        std::cout << std::endl;
    }
    
    void Progress::vertexOrdering(const std::vector<int>& ordering) {
        Printer::vertexOrdering(ordering);
        std::cout << "\rComputing ordering done." << std::flush;
        std::cout << std::endl;
    }

    void Progress::beforeComputation() {
        std::cout << "### Computation ###" << std::endl;
    }
    
    void Progress::solverIntermediateEvent(const htd::vertex_t vertex, const Computation& computation, const std::string& message) {
        int percentSolved = (tdComputedCount * 100) / app.getDecomposition()->vertexCount();
        std::size_t bagSize = app.getDecomposition()->bagSize(vertex);
        std::size_t introduced = app.getDecomposition()->introducedVertexCount(vertex);
        std::size_t removed = app.getDecomposition()->forgottenVertexCount(vertex);
        std::size_t children = app.getDecomposition()->childCount(vertex);
        std::cout << "\r" << std::setw(3) << percentSolved << "% solved. Now solving: TD node " << vertex << ", ";
        std::cout << bagSize << " elements (" << removed << "r, " << introduced << "i), " << children << " child(ren). ";
        std::cout << computation.leavesCount() << " leaves (" << computation.maxBDDsize() << " max BDD) - ";
        std::cout << message << ".                       " << std::endl;
    }

    void Progress::solverIntermediateEvent(const htd::vertex_t vertex, const Computation& c1, const Computation& c2, const std::string& message) {
        int percentSolved = (tdComputedCount * 100) / app.getDecomposition()->vertexCount();
        std::size_t bagSize = app.getDecomposition()->bagSize(vertex);
        std::size_t introduced = app.getDecomposition()->introducedVertexCount(vertex);
        std::size_t removed = app.getDecomposition()->forgottenVertexCount(vertex);
        std::size_t children = app.getDecomposition()->childCount(vertex);
        std::cout << "\r" << std::setw(3) << percentSolved << "% solved. Now solving: TD node " << vertex << ", ";
        std::cout << bagSize << " elements (" << removed << "r, " << introduced << "i), " << children << " child(ren). ";
        std::cout << c1.leavesCount() << " leaves (" << c1.maxBDDsize() << " max BDD) X ";
        std::cout << c2.leavesCount() << " leaves (" << c2.maxBDDsize() << " max BDD) - ";
        std::cout << message << ".                      " << std::endl;
    }

    void Progress::solverInvocationResult(const htd::vertex_t vertex, const Computation& computation) {
        tdComputedCount++;
        int percentSolved = (tdComputedCount * 100) / app.getDecomposition()->vertexCount();
        std::size_t bagSize = app.getDecomposition()->bagSize(vertex);
        std::size_t introduced = app.getDecomposition()->introducedVertexCount(vertex);
        std::size_t removed = app.getDecomposition()->forgottenVertexCount(vertex);
        std::size_t children = app.getDecomposition()->childCount(vertex);
        std::cout << "\r" << std::setw(3) << percentSolved << "% solved. Last solved: TD node " << vertex << ", ";
        std::cout << bagSize << " elements (" << removed << "r, " << introduced << "i), " << children << " child(ren).";
        std::cout << "                                 " << std::endl;
    }

    void Progress::afterComputation() {
        Printer::afterComputation();
        std::cout << "\rSolving done.                        " << std::flush;
        std::cout << std::endl;
    }


} // namespace printer
