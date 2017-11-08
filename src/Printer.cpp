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

#include <algorithm>
#include <functional>

#include <iostream>

#include "Application.h"
#include "Printer.h"
#include "Instance.h"

#include <htd/main.hpp>

Printer::Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault)
: Module(app, app.getPrinterChoice(), optionName, optionDescription, newDefault) {
}

Printer::~Printer() {
}

void Printer::inputInstance(const InstancePtr& instance) {
    if (!app.printInputInstance()) {
        return;
    }

    HTDHypergraph* hypergraph = instance->hypergraph;

    std::cout << "### Input instance ###" << std::endl;
    std::cout << "#quantifiers: " << instance->quantifierCount() << std::endl;
    std::cout << " ";
    for (auto q : instance->getQuantifierSequence()) {
        switch (q) {
            case NTYPE::EXISTS: std::cout << "E ";
                break;
            case NTYPE::FORALL: std::cout << "A ";
                break;
            case NTYPE::UNKNOWN: std::cout << "U ";
                break;
            default: std::cout << "?" << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "#vertices: " << hypergraph->vertexCount() << std::endl;
    for (const auto& vertexId : hypergraph->internalGraph().vertices()) {
        std::cout << " " << vertexId << ": " << hypergraph->vertexName(vertexId);
        int vertexLevel = htd::accessLabel<int>(hypergraph->internalGraph().vertexLabel("level", vertexId));
        NTYPE quantor = instance->quantifier(vertexLevel);
        std::cout << "; level " << vertexLevel << " " << (quantor == NTYPE::EXISTS ? "E" : "A") << std::endl;
    }
    std::cout << "#edges: " << hypergraph->edgeCount() << std::endl;
    for (const auto& hyperedge : hypergraph->internalGraph().hyperedges()) {
        const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(hypergraph->edgeLabel("signs", hyperedge.id()));
        int index = 0;
        std::cout << " " << hyperedge.id() << ": ";
        for (const auto& vertexId : hyperedge.elements()) {
            std::cout << (edgeSigns[index] ? "+" : "-") << hypergraph->vertexName(vertexId) << " ";
            index++;
        }
        std::cout << std::endl;
    }
}

void Printer::preprocessedInstance(const InstancePtr& instance) {
    if (!app.printPreprocessedInstance()) {
        return;
    }

    HTDHypergraph* hypergraph = instance->hypergraph;

    std::cout << "### Preprocessed instance ###" << std::endl;
    std::cout << "#quantifiers: " << instance->quantifierCount() << std::endl;
    std::cout << " ";
    for (auto q : instance->getQuantifierSequence()) {
        switch (q) {
            case NTYPE::EXISTS: std::cout << "E ";
                break;
            case NTYPE::FORALL: std::cout << "A ";
                break;
            case NTYPE::UNKNOWN: std::cout << "U ";
                break;
            default: std::cout << "?" << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "#vertices: " << hypergraph->vertexCount() << std::endl;
    for (const auto& vertexId : hypergraph->internalGraph().vertices()) {
        std::cout << " " << vertexId << ": " << hypergraph->vertexName(vertexId);
        int vertexLevel = htd::accessLabel<int>(hypergraph->internalGraph().vertexLabel("level", vertexId));
        NTYPE quantor = instance->quantifier(vertexLevel);
        std::cout << "; level " << vertexLevel << " " << (quantor == NTYPE::EXISTS ? "E" : "A") << std::endl;
    }
    std::cout << "#edges: " << hypergraph->edgeCount() << std::endl;
    for (const auto& hyperedge : hypergraph->internalGraph().hyperedges()) {
        const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(hypergraph->edgeLabel("signs", hyperedge.id()));
        int index = 0;
        std::cout << " " << hyperedge.id() << ": ";
        for (const auto& vertexId : hyperedge.elements()) {
            std::cout << (edgeSigns[index] ? "+" : "-") << hypergraph->vertexName(vertexId) << " ";
            index++;
        }
        std::cout << std::endl;
    }
}

void Printer::decomposerResult(const HTDDecompositionPtr& result) {
    if (!app.printDecomposition()) {
        return;
    }
    std::cout << "### Tree decomposition ###" << std::endl;
    std::cout << "Width: " << result->maximumBagSize() - 1 << std::endl;
    decomposerResultRec("", result->root());
}

void Printer::decomposerResultRec(const std::string& depth, const htd::vertex_t current) {
    std::cout << depth << current << ": ";
    for (const auto& v : app.getDecomposition()->bagContent(current)) {
        std::cout << app.getInputInstance()->hypergraph->vertexName(v) << " ";
    }
    std::cout << std::endl;

    for (const auto& c : app.getDecomposition()->children(current)) {
        decomposerResultRec(depth + " ", c);
    }
}

void Printer::vertexOrdering(const std::vector<int>& ordering) {
    if (!app.printVertexOrdering()) {
        return;
    }

    std::cout << "### Vertex ordering ###" << std::endl;

    HTDHypergraph* hypergraph = app.getInputInstance()->hypergraph;

    std::vector<htd::vertex_t> verticesSorted(hypergraph->internalGraph().vertices().begin(), hypergraph->internalGraph().vertices().end());

    std::sort(verticesSorted.begin(), verticesSorted.end(), [ordering] (htd::vertex_t x1, htd::vertex_t x2) -> bool {
        int x1Index = ordering[x1];
        int x2Index = ordering[x2];
        return x1Index < x2Index;
    });

    std::cout << "[ ";
    for (const auto vertex : verticesSorted) {
        std::cout << hypergraph->vertexName(vertex) << " "; //":" << ordering[vertex] <<
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
    computation.print(true);
}

void Printer::result(const RESULT result) {
    std::cout << "### Result ###" << std::endl;
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

void Printer::models(const BDD bdd, std::vector<Variable> variables, int limit) {

    HTDHypergraph* hypergraph = app.getInputInstance()->hypergraph;

    // only vertices at level 1
    std::list<Variable> lvl1;
    for (Variable v : variables) {
        int vertexLevel = htd::accessLabel<int>(hypergraph->internalGraph().vertexLabel("level", v.getVertices()[0]));
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

    std::cout << "Sum-of-product cover models (limit " << limit << "):" << std::endl;
    
    int printed = 0;
    modelsRec(bdd, lvl1, model, limit, &printed);
    std::cout << std::endl;
}

void Printer::modelsRec(BDD bdd, std::list<Variable> variables, std::vector<std::string> model, int limit, int* printed) {
    if (limit > 0 && (*printed >= limit)) {
        return;
    }
    
    if (bdd == app.getBDDManager().getManager().bddZero()) {
        return;
    }
    if (variables.empty()) {
        for (std::string literal : model) {
            std::cout << literal << " ";
        }
        std::cout << std::endl;
        if (limit >= 0) {
            (*printed)++;
        }
    } else {
        Variable v = variables.front();
        variables.pop_front();

        HTDHypergraph* hypergraph = app.getInputInstance()->hypergraph;
        // Only works for simple variables! (one type, one number, one vertex)
        vertexNameType vertex = hypergraph->vertexName(v.getVertices()[0]);

        BDD bdd1 = bdd.Restrict(app.getBDDManager().getManager().bddVar(v.getId()));
        BDD bdd0 = bdd.Restrict(!(app.getBDDManager().getManager().bddVar(v.getId())));
        if (bdd1 == bdd0) {
            std::vector<std::string> model10 = model;
            //            model10.push_back("±" + vString);
            modelsRec(bdd1, variables, model10, limit, printed);

        } else {

            std::vector<std::string> model1 = model;
            model1.push_back("+" + std::to_string(vertex));
            modelsRec(bdd1, variables, model1, limit, printed);

            std::vector<std::string> model0 = model;
            model0.push_back("-" + std::to_string(vertex));
            modelsRec(bdd0, variables, model0, limit, printed);
        }
    }
}

void Printer::modelCount(const BDD bdd, std::vector<Variable> variables) {

    HTDHypergraph* hypergraph = app.getInputInstance()->hypergraph;

    // only vertices at level 1
    std::list<Variable> lvl1;
    for (Variable v : variables) {
        int vertexLevel = htd::accessLabel<int>(hypergraph->internalGraph().vertexLabel("level", v.getVertices()[0]));
        if (vertexLevel == 1) {
            lvl1.push_back(v);
        }
    }

    unsigned int variableCount = lvl1.size();

    int digits;

    DdApaNumber minterms = bdd.ApaCountMinterm(variableCount, &digits);

    std::cout << "Model count: " << app.getBDDManager().getManager().ApaStringDecimal(digits, minterms) << std::endl;
    std::cout << std::endl;

    free(minterms);
}