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

#include "htd/main.hpp"
#include "Application.h"

typedef unsigned int vertexNameType;
typedef std::string edgeNameType;
typedef htd::NamedMultiHypergraph<vertexNameType, edgeNameType> HTDHypergraph;

enum NTYPE {
    UNKNOWN, EXISTS, FORALL
};


class Instance {
public:
    Instance(Application& app);
//    Instance(Application& app, std::vector<NTYPE> quantifierSequence);
    ~Instance();

    void pushBackQuantifier(const NTYPE quantifier);
    void pushFrontQuantifier(const NTYPE quantifier);
    const NTYPE innermostQuantifier() const;
    const NTYPE quantifier(const unsigned int level) const;
    const unsigned int quantifierCount() const;
    const std::vector<NTYPE> getQuantifierSequence() const;
    
    HTDHypergraph* hypergraph;
    

protected:
    Application& app;
    
private:
    std::vector<NTYPE> quantifierSequence;
};
