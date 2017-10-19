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

#include "options/OptionHandler.h"
#include "options/Choice.h"
#include "Utils.h"
#include "options/DefaultIntegerValueOption.h"

#include <mtr.h>

#include <cuddObj.hh>

#include <htd/NamedMultiHypergraph.hpp>
#include <htd/IMutableHypertreeDecomposition.hpp>

enum RESULT {
    SAT, UNSAT, UNDECIDED
};

#define RETURN_SAT 10
#define RETURN_UNSAT 20
#define RETURN_UNDECIDED 1
#define RETURN_UNFINISHED 2


class HGInputParser;
class Decomposer;
class Preprocessor;
class Ordering;
class SolverFactory;
class Printer;
class BDDManager;
class Instance;
class ComputationManager;

typedef std::shared_ptr<Instance> InstancePtr;
typedef std::shared_ptr<htd::IMutableTreeDecomposition> HTDDecompositionPtr;


class Application {
public:
    
    Application(const std::string& binaryName);
    ~Application();

    // We assume that argv[0] contains the first option, NOT the binary name
    int run(int argc, char** argv);
    
    // Print usage (but don't exit)
    void usage() const;
    void version() const;
    void license() const;

    options::OptionHandler& getOptionHandler();
    options::Choice& getHGInputParserChoice();
    options::Choice& getDecomposerChoice();
    options::Choice& getPreprocessorChoice();
    options::Choice& getOrderingChoice();
    options::Choice& getSolverChoice();
    options::Choice& getPrinterChoice();
    
    InstancePtr getInputInstance() const;
    const std::vector<int>& getVertexOrdering() const;
    HTDDecompositionPtr getDecomposition() const;
    const SolverFactory& getSolverFactory() const;
    Printer& getPrinter() const;

    void setHGInputParser(HGInputParser& inputParser);
    void setDecomposer(Decomposer& decomposer);
    void setPreprocessor(Preprocessor& preprocessor);
    void setOrdering(Ordering& ordering);
    void setSolverFactory(SolverFactory& solverFactory);
    void setPrinter(Printer& printer);

    bool printInputInstance() const;
    bool printPreprocessedInstance() const;
    bool printDecomposition() const;
    bool printVertexOrdering() const;
    bool enumerate() const;
    int enumerateLimit() const;
    bool modelCount() const;

    BDDManager& getBDDManager() const;
    ComputationManager& getNSFManager() const;
    htd::LibraryInstance* getHTDManager() const;
    Decomposer& getDecomposer() const;

private:
    
    static const std::string MODULE_SECTION;

    std::string binaryName;
    
    InstancePtr inputInstance;
    std::vector<int> vertexOrdering;

    HTDDecompositionPtr decomposition;
    
    options::OptionHandler opts;
    options::Option optHelp;
    options::Option optVersion;
    options::Option optLicense;
    options::SingleValueOption optInputFile;
    options::Choice optHGInputParser;
    options::Choice optDecomposer;
    options::Choice optPreprocessor;
    options::Choice optOrdering;
    options::Choice optSolver;
    options::Choice optPrinter;
    options::Option optPrintInputInstance;
    options::Option optPrintPreprocessedInstance;
    options::Option optPrintDecomposition;
    options::Option optPrintVertexOrdering;
    options::Option optOnlyParseInstance;
    options::Option optOnlyDecomposeInstance;
    options::DefaultIntegerValueOption optEnumerate;
    options::Option optModelCount;
    options::SingleValueOption optSeed;

    HGInputParser* hgInputParser;
    Decomposer* decomposer;
    Preprocessor* preprocessor;
    Ordering* ordering;
    SolverFactory* solverFactory;
    Printer* printer;

    BDDManager* bddManager;
    ComputationManager* nsfManager;
    htd::LibraryInstance* htdManager;
};
