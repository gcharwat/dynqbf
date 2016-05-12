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

    HTDHypergraphPtr DIMACSDriver::parse(std::istream& input) {
        NSFManager& nsfMan = app.getNSFManager();

        HTDHypergraphPtr hypergraph(new htd::NamedMultiHypergraph<std::string, std::string>());

        std::string inputFileFormat, dummy;
        unsigned int atoms = 0, clauses = 0;
        unsigned int firstLevelCount = 0;

        std::string line;
        std::string lineElement;

        while (getline(input, line)) {
            if (line.length() == 0) continue;
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
                    if (nsfMan.innermostQuantifier() != NTYPE::EXISTS) {
                        nsfMan.pushBackQuantifier(NTYPE::EXISTS);
                    }
                    std::getline(lineStream, lineElement, ' '); // skip first character
                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "0") break;
                        hypergraph->addVertex(lineElement);
                        hypergraph->setVertexLabel("level", lineElement, new htd::Label<int>(nsfMan.quantifierCount()));
                        if (nsfMan.quantifierCount() == 1) {
                            firstLevelCount++;
                        }
                    }
                    break;
                case 'a':
                    if (nsfMan.innermostQuantifier() != NTYPE::FORALL) {
                        nsfMan.pushBackQuantifier(NTYPE::FORALL);
                    }
                    std::getline(lineStream, lineElement, ' '); // skip first character
                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "0") break;
                        hypergraph->addVertex(lineElement);
                        hypergraph->setVertexLabel("level", lineElement, new htd::Label<int>(nsfMan.quantifierCount()));
                        if (nsfMan.quantifierCount() == 1) {
                            firstLevelCount++;
                        }
                    }
                    break;
                default:
                    if (clauses == 0) error("invalid number of clauses");
                    std::vector<bool> signs;
                    std::vector<std::string> atoms;

                    while (std::getline(lineStream, lineElement, ' ')) {
                        if (lineElement == "0") break;
                        if (lineElement.length() == 0) continue;
                        std::string vertexName;
                        bool sign = true;
                        if (lineElement.at(0) == '-') {
                            sign = false;
                            vertexName = lineElement.substr(1, lineElement.length());
                        } else {
                            vertexName = lineElement;
                        }
                        // Does the vertex already exist?
                        // TODO adding vertex also checks if it already exists -> remove redundant check
                        if (!hypergraph->isVertexName(vertexName)) {
                            // Shift all levels by 1
                            if (nsfMan.quantifier(1) != NTYPE::EXISTS) {
                                nsfMan.pushFrontQuantifier(NTYPE::EXISTS);
                                for (const auto& oldVertex : hypergraph.get()->internalGraph().vertices()) {
                                    const std::string oldVertexName = hypergraph->vertexName(oldVertex);
                                    int oldVertexLevel = htd::accessLabel<int>(hypergraph->internalGraph().vertexLabel("level", oldVertex));
                                    int newVertexLevel = oldVertexLevel + 1;
                                    hypergraph->setVertexLabel("level", oldVertexName, new htd::Label<int>(newVertexLevel));
                                }
                            }
                            hypergraph->addVertex(vertexName);
                            hypergraph->setVertexLabel("level", vertexName, new htd::Label<int>(1)); // insert at level 1
                            firstLevelCount++;
                        }
                        atoms.push_back(vertexName);
                        signs.push_back(sign);
                    }
                    // immediately abort
                    if (atoms.size() == 0) {
                        throw AbortException("empty clause in input instance", RESULT::UNSAT);
                    }
                    htd::id_t edgeId = hypergraph->addEdge(atoms, std::to_string(clauses));
                    hypergraph->setEdgeLabel("signs", edgeId, new htd::Label < std::vector<bool>>(signs));
                    clauses--;
            }
        }
        if (hypergraph->vertexCount() == 0) {
            throw AbortException("empty input instance", RESULT::SAT);
        }

        return hypergraph;
    }

    void DIMACSDriver::error(const std::string& m) {
        std::ostringstream str;
        str << "Error reading DIMACS file (" << m << ')';
        throw std::runtime_error(str.str());
    }

} // namespace parser
