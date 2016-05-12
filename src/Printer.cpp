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

#include <algorithm>
#include <functional>

#include <iostream>

#include "Application.h"
#include "Printer.h"

#include <htd/main.hpp>

Printer::Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault)
: Module(app, app.getPrinterChoice(), optionName, optionDescription, newDefault) {
}

Printer::~Printer() {
}

void Printer::inputHypergraph(const HTDHypergraphPtr& hypergraph) {
    if (!app.printInputHypergraph()) {
        return;
    }

    std::cout << "#vertices: " << hypergraph.get()->vertexCount() << std::endl;
    for (const auto& vertexId : hypergraph.get()->internalGraph().vertices()) {
        std::cout << " " << vertexId << ": " << hypergraph.get()->vertexName(vertexId) << "\t";
        int vertexLevel = htd::accessLabel<int>(app.getInputHypergraph()->internalGraph().vertexLabel("level", vertexId));
        std::cout << "level " << vertexLevel << std::endl;
    }
    std::cout << "#edges: " << hypergraph.get()->edgeCount() << std::endl;
    for (const auto& hyperedge : hypergraph.get()->internalGraph().hyperedges()) {
        std::cout << " " << hyperedge.id() << ": ";
        for (const auto& vertexId : hyperedge.elements()) {
            std::cout << hypergraph.get()->vertexName(vertexId) << " ";
        }
        std::cout << std::endl;
    }
}

void Printer::preprocessedHypergraph(const HTDHypergraphPtr& hypergraph) {
    inputHypergraph(hypergraph);
}

void Printer::decomposerResult(const HTDDecompositionPtr& result) {
    if (!app.printDecomposition()) {
        return;
    }
    std::cout << "Width: " << (result->maximumBagSize() - 1) << std::endl;
}

void Printer::vertexOrdering(const std::vector<int>& ordering) {
    if (!app.printVertexOrdering()) {
        return;
    }

    std::vector<htd::vertex_t> verticesSorted(app.getInputHypergraph()->internalGraph().vertices().begin(), app.getInputHypergraph()->internalGraph().vertices().end());

    std::sort(verticesSorted.begin(), verticesSorted.end(), [ordering] (htd::vertex_t x1, htd::vertex_t x2) -> bool {
        int x1Index = ordering[x1];
        int x2Index = ordering[x2];
        return x1Index < x2Index;
    });


    std::cout << "[ ";
    for (const auto vertex : verticesSorted) {
        std::cout << app.getInputHypergraph()->vertexName(vertex) << " "; //":" << ordering[vertex] <<
    }
    std::cout << "]" << std::endl;
}

void Printer::solverInvocationResult(const htd::vertex_t vertex, const Computation& computation) {
}

void Printer::solverIntermediateEvent(const htd::vertex_t vertex, const Computation& computation, const std::string& message) {
}

void Printer::solverIntermediateEvent(const htd::vertex_t vertex, const Computation& c1, const Computation& c2, const std::string& message) {    
}

void Printer::beforeComputation() {
}

void Printer::resultComputation(const Computation& computation) {
    std::cout << "Computation: ";
    computation.printCompact();

}

void Printer::result(const RESULT result) {
    std::cout << "Solution: ";
    switch (result) {
        case SAT:
            std::cout << "SAT" << std::endl;
            break;
        case UNSAT:
            std::cout << "UNSAT" << std::endl;
            break;
        case UNDECIDED:
            std::cout << "UNDECIDED" << std::endl;
            break;
    }

}

void Printer::afterComputation() {
}

void Printer::select() {
    Module::select();
    app.setPrinter(*this);
    app.getVertexOrdering();
}

void Printer::models(const BDD bdd, std::vector<Variable> variables) {
    // only vertices at level 1
    std::list<Variable> lvl1;
    for (Variable v : variables) {
        int vertexLevel = htd::accessLabel<int>(app.getInputHypergraph()->internalGraph().vertexLabel("level", v.getVertices()[0]));
        if (vertexLevel == 1) {
            lvl1.push_back(v);
        }
    }

    lvl1.sort([this] (Variable v1, Variable v2) -> bool {
            int ind1 = app.getVertexOrdering()[v1.getVertices()[0]];
            int ind2 = app.getVertexOrdering()[v2.getVertices()[0]];
            return (ind1 < ind2);
        });


    std::vector<std::string> model;

    modelsRec(bdd, lvl1, model);

}

void Printer::modelsRec(BDD bdd, std::list<Variable> variables, std::vector<std::string> model) {
    if (bdd == app.getBDDManager().getManager().bddZero()) {
        return;
    }
    if (variables.empty()) {
        for (std::string literal : model) {
            std::cout << literal << " ";
        }
        std::cout << std::endl;
    } else {
        Variable v = variables.front();
        variables.pop_front();

        // Only works for simple variables! (one type, one number, one vertex)
        std::string vString = app.getInputHypergraph()->vertexName(v.getVertices()[0]);

        BDD bdd1 = bdd.Restrict(app.getBDDManager().getManager().bddVar(v.getId()));
        BDD bdd0 = bdd.Restrict(!(app.getBDDManager().getManager().bddVar(v.getId())));
        if (bdd1 == bdd0) {
            std::vector<std::string> model10 = model;
//            model10.push_back("±" + vString);
            modelsRec(bdd1, variables, model10);

        } else {

            std::vector<std::string> model1 = model;
            model1.push_back("+" + vString);
            modelsRec(bdd1, variables, model1);

            std::vector<std::string> model0 = model;
            model0.push_back("-" + vString);
            modelsRec(bdd0, variables, model0);
        }
    }

}