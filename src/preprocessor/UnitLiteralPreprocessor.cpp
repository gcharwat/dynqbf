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

#include <cassert>

#include "UnitLiteralPreprocessor.h"
#include "../Application.h"

#include <htd/main.hpp>
#include <limits.h>


namespace preprocessor {

    UnitLiteralPreprocessor::UnitLiteralPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "unit-literal", "apply unit literal elimination", newDefault) {
    }

    HTDHypergraphPtr UnitLiteralPreprocessor::preprocess(const HTDHypergraphPtr& instance) const {
        std::vector<htd::vertex_t> unitLiterals;
        std::vector<bool> unitLiteralSigns;

        for (auto clause : instance->internalGraph().hyperedges()) {
            bool unit = false;

            const std::vector<bool> &edgeSigns = htd::accessLabel < std::vector<bool>>(instance->edgeLabel("signs", clause.id()));

            htd::vertex_t existentialVariable = htd::Vertex::UNKNOWN;
            unsigned int existsLevel = 0;
            bool existsSign = false;
            unsigned int minForallLevel = UINT_MAX;
            unsigned int index = 0;
            for (auto variable : clause) {
                unsigned int variableLevel = htd::accessLabel<int>(instance->internalGraph().vertexLabel("level", variable));
                NTYPE quantor = app.getNSFManager().quantifier(variableLevel);
                if (quantor == NTYPE::EXISTS) {
                    if (existentialVariable != htd::Vertex::UNKNOWN) {
                        existentialVariable = variable;
                        existsLevel = variableLevel;
                        existsSign = edgeSigns[index];
                        unit = true;
                    } else {
                        unit = false;
                        break; // more than 1 existential variable in clause
                    }
                } else {
                    if (variableLevel < minForallLevel) {
                        minForallLevel = variableLevel;
                    }
                }
                if (minForallLevel < existsLevel) {
                    unit = false;
                    break; // universal is at left of exists
                }
                index++;
            }
            if (unit) {
                unitLiterals.push_back(existentialVariable);
                unitLiteralSigns.push_back(existsSign);
            }
        }

        std::cout << "Unit: ";
        for (auto variable : unitLiterals) {
            std::cout << instance->vertexName(variable) << " ";
        }
        std::cout << std::endl;
        
        return instance;
    }

} // namespace preprocessor
