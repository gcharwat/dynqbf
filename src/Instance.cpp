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

#include "Application.h"
#include "Instance.h"

Instance::Instance(Application& app)
: app(app) {
    hypergraph = new HTDHypergraph(app.getHTDManager());
}

Instance::~Instance() {
    delete hypergraph;
}

void Instance::pushBackQuantifier(const NTYPE quantifier) {
    quantifierSequence.push_back(quantifier);
}

void Instance::pushFrontQuantifier(const NTYPE quantifier) {
    quantifierSequence.insert(quantifierSequence.begin(), quantifier);
}

const NTYPE Instance::innermostQuantifier() const {
    return quantifier(quantifierCount());
}

const NTYPE Instance::quantifier(const unsigned int level) const {
    if (level < 1 || level > quantifierCount()) {
        return NTYPE::UNKNOWN;
    }
    return quantifierSequence[level - 1];
}

const unsigned int Instance::quantifierCount() const {
    return quantifierSequence.size();
}

const std::vector<NTYPE> Instance::getQuantifierSequence() const {
    return quantifierSequence;
}