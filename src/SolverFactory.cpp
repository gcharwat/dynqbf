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

#include <string>
#include <vector>

#include "SolverFactory.h"
#include "Application.h"

SolverFactory::SolverFactory(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault)
: Module(app, app.getSolverChoice(), optionName, optionDescription, newDefault) {
}

void SolverFactory::select() {
    Module::select();
    app.setSolverFactory(*this);
}

BDD SolverFactory::getBDDVariable(const std::string& type, const int position, const std::vector<htd::vertex_t>& vertices) const {
    throw std::runtime_error("No variables specified");
}

std::vector<Variable> SolverFactory::getVariables() const {
    return std::vector<Variable>();
}
