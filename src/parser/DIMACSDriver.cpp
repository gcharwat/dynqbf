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

#include "../Utils.h"
#include "../AbortException.h"
#include "DIMACSDriver.h"

#include <map>

#include <sstream>
#include <iostream>

#include <htd/ILabel.hpp>
#include <stack>

namespace parser {

    DIMACSDriver::DIMACSDriver(Application& app, bool newDefault)
    : HGInputParser(app, "dimacs", "read (Q)DIMACS file", newDefault) {
    }

    DIMACSDriver::~DIMACSDriver() {
    }

    InstancePtr DIMACSDriver::parse(std::istream& input) {
        InstancePtr instance(new Instance(app));

        std::string inputFileFormat, dummy;
        unsigned int atoms = 0, clauses = 0;
        htd::vertex_t maxVertexId = htd::Vertex::FIRST - 1;
        //        unsigned int firstLevelCount = 0;

        std::string line;
        std::string lineElement;

        while (getline(input, line)) {
            if (line.length() == 0) continue;
            if (line[line.length()-1] == '\r') line = line.substr(0, line.length()-1); // fix for files with \r\n line ending
            char firstChar = line.at(0);
            std::istringstream lineStream(line);
            switch (firstChar) {
                case 'c': break;
                case 'p':
                    if (lineStream >> dummy >> inputFileFormat >> atoms >> clauses) {
                        if ("cnf" != inputFileFormat) error("not a cnf file");
                    }
                    break;
                case 'e':
                    if (instance->innermostQuantifier() != NTYPE::EXISTS) {
                        instance->pushBackQuantifier(NTYPE::EXISTS);
                    }
                    std::getline(lineStream, lineElement, ' '); // skip first character
                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "") continue;
                        if (lineElement == "0") break;
                        vertexNameType vertex = utils::strToInt(lineElement, "Error converting element");
                        htd::vertex_t vertexId = instance->hypergraph->addVertex(vertex);
                        if (vertexId > maxVertexId) {
                            maxVertexId = vertexId;
                        }
                        instance->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(instance->quantifierCount()));
                    }
                    break;
                case 'a':
                    if (instance->innermostQuantifier() != NTYPE::FORALL) {
                        instance->pushBackQuantifier(NTYPE::FORALL);
                    }
                    std::getline(lineStream, lineElement, ' '); // skip first character
                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "") continue;
                        if (lineElement == "0") break;
                        vertexNameType vertex = utils::strToInt(lineElement, "Error converting element");
                        htd::vertex_t vertexId = instance->hypergraph->addVertex(vertex);
                        if (vertexId > maxVertexId) {
                            maxVertexId = vertexId;
                        }
                        instance->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(instance->quantifierCount()));
                    }
                    break;
                default:
                    if (clauses == 0) error("invalid number of clauses");
                    std::vector<bool> signs;
                    std::vector<vertexNameType> atoms;

                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "0") break;
                        if (lineElement == "") continue;
                        int parsedLiteral = utils::strToInt(lineElement, "Error converting element");
                        
                        bool sign = true;
                        if (parsedLiteral < 0) {
                            sign = false;
                            parsedLiteral *= -1;
                        } 
                        vertexNameType vertex = parsedLiteral;
                        
                        // Does the vertex already exist?
                        htd::vertex_t vertexId = instance->hypergraph->addVertex(vertex);
                        if (vertexId > maxVertexId) {
                            maxVertexId = vertexId;
                            // if not, shift all levels by 1
                            if (instance->quantifier(1) != NTYPE::EXISTS) {
                                instance->pushFrontQuantifier(NTYPE::EXISTS);
                                for (const auto& oldVertexId : instance->hypergraph->internalGraph().vertices()) {
                                    if (oldVertexId != vertexId) {
                                        const vertexNameType oldVertex = instance->hypergraph->vertexName(oldVertexId);
                                        int oldVertexLevel = htd::accessLabel<int>(instance->hypergraph->internalGraph().vertexLabel("level", oldVertexId));
                                        int newVertexLevel = oldVertexLevel + 1;
                                        instance->hypergraph->setVertexLabel("level", oldVertex, new htd::Label<int>(newVertexLevel));
                                    }
                                }
                            }
                            instance->hypergraph->setVertexLabel("level", vertex, new htd::Label<int>(1)); // insert at level 1
                        }
                        atoms.push_back(vertex);
                        signs.push_back(sign);
                    }
                    // immediately abort
                    if (atoms.size() == 0) {
                        throw AbortException("empty clause in input instance", RESULT::UNSAT);
                    }
                    htd::id_t edgeId = instance->hypergraph->addEdge(atoms, std::to_string(clauses));
                    instance->hypergraph->setEdgeLabel("signs", edgeId, new htd::Label < std::vector<bool>>(signs));
                    clauses--;
            }
        }
        if (instance->hypergraph->vertexCount() == 0) {
            throw AbortException("empty input instance", RESULT::SAT);
        }

        return instance;
    }

    void DIMACSDriver::error(const std::string& m) {
        std::ostringstream str;
        str << "Error reading DIMACS file (" << m << ')';
        throw std::runtime_error(str.str());
    }

} // namespace parser
