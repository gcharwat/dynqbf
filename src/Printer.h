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

#pragma once

#include "Module.h"
#include "cuddObj.hh"
#include "SolverFactory.h"
#include "Application.h"

class Printer : public Module {
public:
    Printer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);
    virtual ~Printer() = 0;

    virtual void inputInstance(const InstancePtr& instance); // the parsed instance
    virtual void preprocessedInstance(const InstancePtr& instance); // the preprocessed instance
    virtual void decomposerResult(const HTDDecompositionPtr& result); // The tree decomposition
    virtual void vertexOrdering(const std::vector<int>& ordering); // The vertex ordering

    virtual void beforeComputation();
    virtual void solverIntermediateEvent(const htd::vertex_t vertex, const Computation& computation, const std::string& message);
    virtual void solverIntermediateEvent(const htd::vertex_t vertex, const Computation& c1, const Computation& c2, const std::string& message);
    virtual void solverInvocationResult(const htd::vertex_t vertex, const Computation& computation);
    virtual void afterComputation();
    virtual void resultComputation(const Computation& computation);
    virtual void result(const RESULT result);

    virtual void models(const BDD bdd, const std::vector<Variable> variables, int limit);
    virtual void modelCount(const BDD bdd, const std::vector<Variable> variables);
    
    virtual void select() override;

private:
    void decomposerResultRec(const std::string& depth, const htd::vertex_t current);
    void modelsRec(BDD bdd, std::list<Variable> variables, std::vector<std::string> model, int limit, int* printed);
};
