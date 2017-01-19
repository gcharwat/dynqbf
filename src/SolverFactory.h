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

#include <memory>

#include "options/Option.h"
#include "Module.h"
#include "Solver.h"
#include "cuddObj.hh"
#include "Variable.h"
#include "Application.h"

#include <htd/Vertex.hpp>

class SolverFactory : public Module {
public:
    SolverFactory(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);

    virtual std::unique_ptr<Solver> newSolver() const = 0;

    virtual void select() override;

    virtual BDD getBDDVariable(const std::string& type, const int position = 0, const std::vector<htd::vertex_t>& vertices = {}) const;
    virtual std::vector<Variable> getVariables() const; // get all used variables

    static const std::string OPTION_SECTION;


};
