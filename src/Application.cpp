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

#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>

#include "Utils.h"
#include "DynQBFConfig.h"
#include "Application.h"
#include "BDDManager.h"
#include "nsf/Computation.h"
#include "nsf/ComputationManager.h"
#include "AbortException.h"

#include "options/MultiValueOption.h"
#include "options/SingleValueOption.h"
#include "options/OptionHandler.h"
#include "options/HelpObserver.h"

#include "parser/DIMACSDriver.h"
//#include "parser/DIMACSIncidenceDriver.h"

#include "Preprocessor.h"
#include "preprocessor/NoPreprocessor.h"
#include "preprocessor/UnitLiteralPreprocessor.h"
#include "preprocessor/CNF3Preprocessor.h"
#include "preprocessor/SplitPreprocessor.h"
#include "preprocessor/CombinedPreprocessor.h"

#include "decomposer/HTDTreeDecomposer.h"
#include "decomposer/SingleNodeDecomposer.h"

#include "ordering/LexicographicalOrdering.h"
#include "ordering/InstanceOrdering.h"
#include "ordering/LevelOrdering.h"
#include "ordering/LevelInverseOrdering.h"
#include "ordering/MinCutOrdering.h"
#include "ordering/MaxBagOrdering.h"
#include "ordering/MaxClauseOrdering.h"
#include "ordering/MinDegreeOrdering.h"

#include "solver/dummy/SolverFactory.h"
#include "solver/bdd/qsat/QSatCNFEDMSolverFactory.h"
#include "solver/bdd/qsat/QSatCNFLDMSolverFactory.h"
#include "solver/bdd/qsat/QSat2CNFSolverFactory.h"
#include "solver/bdd/qsat/QSatBDDSolverFactory.h"

#include "printer/Quiet.h"
#include "printer/Progress.h"
#include "printer/Debug.h"
#include "printer/Verbose.h"
#include "printer/Performance.h"

const std::string Application::MODULE_SECTION = "Module selection";

Application::Application(const std::string& binaryName)
: binaryName(binaryName)
, optHelp("h", "Print usage information and exit")
, optVersion("v", "Print version information and exit")
, optLicense("l", "Print license information and exit")
, optInputFile("f", "file", "Read problem instance from <file> (default: from stdin)")
, optHGInputParser("i", "input-format", "Specify the instance input format")
, optDecomposer("d", "decomposer", "Use decomposition method <decomposer>")
, optPreprocessor("r", "preprocessor", "Use instance preprocessor <preprocessor>")
, optOrdering("o", "ordering", "Initially use BDD variable ordering <ordering>")
, optSolver("p", "problem-solver", "Use <problem-solver> to solve problem")
, optPrinter("output", "printer", "Print information during the run using <printer>")
, optPrintInputInstance("print-instance", "Print the input hypergraph")
, optPrintPreprocessedInstance("print-preprocessed", "Print the preprocessed hypergraph")
, optPrintDecomposition("print-decomposition", "Print the computed decomposition")
, optPrintVertexOrdering("print-ordering", "Print the computed initial vertex ordering")
, optOnlyParseInstance("only-parse-instance", "Only construct hypergraph and exit")
, optOnlyDecomposeInstance("only-decompose", "Only parse input instance, decompose it and exit")
, optEnumerate("enumerate", "l", "Enumerate <l> (sum-of-product cover) models (for outermost quantifier block. if existential and SAT: model; if universal and UNSAT: 'forwhich' model), 0 to enumerate all", 1)
, optModelCount("model-count", "Count models (for outermost quantifier block. if existential and SAT: model count; if universal and UNSAT: 'forwhich' model count)")
, optSeed("seed", "s", "Initialize random number generator with seed <s>") {
}

Application::~Application() {
    if (nsfManager != NULL) {
        delete nsfManager;
    } 
    if (bddManager != NULL) {
        delete bddManager;
    }
    if (htdManager != NULL) {
        htdManager->terminate();
        delete htdManager;
    }
}

int Application::run(int argc, char** argv) {

    opts.addOption(optHelp);
    options::HelpObserver helpObserver(*this, optHelp);
    opts.registerObserver(helpObserver);
    opts.addOption(optVersion);
    opts.addOption(optLicense);

    opts.addOption(optInputFile);
    opts.addOption(optOnlyParseInstance);
    opts.addOption(optOnlyDecomposeInstance);
    opts.addOption(optPrintInputInstance);
    opts.addOption(optPrintPreprocessedInstance);
    opts.addOption(optPrintDecomposition);
    opts.addOption(optPrintVertexOrdering);
    opts.addOption(optEnumerate);
    opts.addOption(optModelCount);
    opts.addOption(optSeed);

    //opts.addOption(optHGInputParser, MODULE_SECTION); // uncomment to add to selection
    parser::DIMACSDriver dimacsParser(*this, true);
    //parser::DIMACSIncidenceDriver dimacsIncidenceParser(*this);

    opts.addOption(optPreprocessor, MODULE_SECTION);
    preprocessor::SplitPreprocessor splitPreprocessor(*this, true);
    preprocessor::NoPreprocessor noPreprocessor(*this);
    // preprocessor::UnitLiteralPreprocessor unitLiteralPreprocessor(*this);
    preprocessor::CNF3Preprocessor cnf3Preprocessor(*this);
    // preprocessor::CombinedPreprocessor combinedPreprocessor(*this);


    opts.addOption(optSolver, MODULE_SECTION);
    solver::bdd::qsat::QSatCNFEDMSolverFactory qsatSolverCNFEDMFactory(*this, true);
    solver::bdd::qsat::QSatCNFLDMSolverFactory qsatSolverCNFLDMFactory(*this);
    //solver::bdd::qsat::QSat2CNFSolverFactory qsat2SolverCNFFactory(*this);
    solver::bdd::qsat::QSatBDDSolverFactory qsatSolverBDDFactory(*this);
    // solver::dummy::SolverFactory dummySolverFactory(*this);

    opts.addOption(optDecomposer, MODULE_SECTION);
    decomposer::HTDTreeDecomposer treeDecomposer(*this, true);
    decomposer::SingleNodeDecomposer singleNodeDecomposer(*this);

    opts.addOption(optOrdering, MODULE_SECTION);
    ordering::InstanceOrdering instanceOrdering(*this, true);
    ordering::LevelOrdering levelOrdering(*this);
    ordering::LevelInverseOrdering levelInverseOrdering(*this);
    ordering::LexicographicalOrdering lexicographicalOrdering(*this);
    ordering::MinDegreeOrdering minDegreeOrdering(*this);
    ordering::MinCutOrdering minCutOrdering(*this);
    ordering::MaxBagOrdering maxBagOrdering(*this);
    ordering::MaxClauseOrdering maxClauseOrdering(*this);

    opts.addOption(optPrinter, MODULE_SECTION);
    printer::Quiet quietPrinter(*this, true);
    printer::Progress progressPrinter(*this);
    printer::Debug debugPrinter(*this);
    printer::Verbose verbosePrinter(*this);
    printer::Performance performancePrinter(*this);

    bddManager = new BDDManager(*this);
    nsfManager = new ComputationManager(*this);
    htdManager = htd::createManagementInstance(htd::Id::FIRST);

    time_t seed = time(0);
    // Parse command line
    try {
        opts.parse(argc, argv);
        opts.checkConditions();
        if (optSeed.isUsed())
            seed = utils::strToInt(optSeed.getValue(), "Invalid random seed");
    } catch (...) {
        usage();
        throw;
    }

    if (optVersion.isUsed()) {
        version();
        return RESULT::UNDECIDED;
    }
    
    if (optLicense.isUsed()) {
        license();
        return RESULT::UNDECIDED;
    }

    srand(seed);

    RESULT result = RESULT::UNDECIDED;

    try {
        // Parse instance
        std::unique_ptr<std::istream> input;
        if (optInputFile.isUsed()) {
            input.reset(new std::ifstream(optInputFile.getValue()));
            if (!input->good()) {
                throw std::runtime_error("Error reading input file");
            }
        }
        inputInstance = hgInputParser->parse(input ? *input : std::cin);
        printer->inputInstance(inputInstance);
        if (optOnlyParseInstance.isUsed()) {
            return RETURN_UNFINISHED;
        }

        // Preprocess instance
        inputInstance = preprocessor->preprocess(inputInstance);
        printer->preprocessedInstance(inputInstance);

        // Decompose instance
        decomposition = decomposer->decompose(inputInstance);
        printer->decomposerResult(decomposition);
        if (optOnlyDecomposeInstance.isUsed()) {
            return RETURN_UNFINISHED;
        }

        vertexOrdering = ordering->computeVertexOrder(inputInstance, decomposition);
        printer->vertexOrdering(vertexOrdering);

        // Initialize CUDD manager
        bddManager->init(inputInstance->hypergraph->vertexCount());

        // Solve the problem
        printer->beforeComputation();
        std::unique_ptr<Solver> solver = solverFactory->newSolver();
        Computation* computation = solver->compute(decomposition->root());

        // Return result
        result = nsfManager->decide(*computation);
        if (//(result == SAT) && 
                enumerate()) {
            BDD answer = nsfManager->solutions(*computation);
            printer->models(answer, solverFactory->getVariables(), enumerateLimit());
        }
        if (modelCount()) {
            BDD answer = nsfManager->solutions(*computation);
            printer->modelCount(answer, solverFactory->getVariables());
        }

        delete computation;

    } catch (AbortException e) {
        result = e.getResult();
        std::cout << "Notice: " << e.what() << std::endl;
    }

    int exitCode = RETURN_UNFINISHED;

    switch (result) {
        case SAT:
            exitCode = RETURN_SAT;
            break;
        case UNSAT:
            exitCode = RETURN_UNSAT;
            break;
        case UNDECIDED:
            exitCode = RETURN_UNDECIDED;
            break;
    }

    printer->afterComputation();

    printer->result(result);

    decomposition.reset();
    inputInstance.reset();

    return exitCode;
}

void Application::usage() const {
    std::cerr << "Usage: " << binaryName << " [options] < instance" << std::endl;
    opts.printHelp();
}

void Application::version() const {
    std::cerr << "Version:            " << DYNQBF_VERSION_MAJOR << "." << DYNQBF_VERSION_MINOR << "." << DYNQBF_VERSION_PATCH << std::string(DYNQBF_VERSION_PRERELEASE) << std::endl;
    std::cerr << "Github Commit ID:   " << DYNQBF_GIT_COMMIT_ID << std::endl;
    std::cerr << " with HTD Commit:   " << HTD_GIT_COMMIT_ID << std::endl;
    std::cerr << "DepQBF integration: ";
#ifdef DEPQBF_ENABLED
    std::cerr << "yes" << std::endl;
#else
    std::cerr << "no" << std::endl;
#endif
    std::cerr << "Built on:           " << __DATE__ << " at " << __TIME__ << std::endl;
}

void Application::license() const {
    std::cout << std::endl;
    std::cout << "dynQBF is released under the GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007" << std::endl;
    std::cout << " A copy of the license should be provided with the system, otherwise see" << std::endl;
    std::cout << " http://www.gnu.org/licenses/" << std::endl << std::endl;
    
    std::cerr << "dynQBF uses the following external libraries:" << std::endl << std::endl;
    
    std::cerr << "- HTD (see https://github.com/mabseher/htd)" << std::endl;
    std::cerr << "  HTD is is released under the GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007" << std::endl << std::endl;
    
#ifdef DEPQBF_ENABLED
    std::cerr << "- DepQBF (https://github.com/lonsing/depqbf)" << std::endl;
    std::cerr << "  DepQBF is is released under the GNU GENERAL PUBLIC LICENSE  Version 3, 29 June 2007" << std::endl << std::endl;
#endif
    
    std::cerr << "- CUDD (http://vlsi.colorado.edu/~fabio/CUDD/)" << std::endl;
    std::cerr << "  CUDD is released under the following license:" << std::endl << std::endl;
    
    std::cerr << "    Copyright (c) 1995-2004, Regents of the University of Colorado      " << std::endl;
    std::cerr << "    All rights reserved.                                                " << std::endl;
    std::cerr << "                                                                        " << std::endl;
    std::cerr << "    Redistribution and use in source and binary forms, with or without  " << std::endl;
    std::cerr << "    modification, are permitted provided that the following conditions  " << std::endl;
    std::cerr << "    are met:                                                            " << std::endl;
    std::cerr << "                                                                        " << std::endl;
    std::cerr << "    Redistributions of source code must retain the above copyright      " << std::endl;
    std::cerr << "    notice, this list of conditions and the following disclaimer.       " << std::endl;
    std::cerr << "                                                                     " << std::endl;
    std::cerr << "    Redistributions in binary form must reproduce the above copyright   " << std::endl;
    std::cerr << "    notice, this list of conditions and the following disclaimer in the " << std::endl;
    std::cerr << "    documentation and/or other materials provided with the distribution." << std::endl;
    std::cerr << "                                                                        " << std::endl;
    std::cerr << "    Neither the name of the University of Colorado nor the names of its " << std::endl;
    std::cerr << "    contributors may be used to endorse or promote products derived from" << std::endl;
    std::cerr << "    this software without specific prior written permission.            " << std::endl;
    std::cerr << "                                                                        " << std::endl;
    std::cerr << "    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS " << std::endl;
    std::cerr << "    \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT " << std::endl;
    std::cerr << "    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS   " << std::endl;
    std::cerr << "    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE      " << std::endl;
    std::cerr << "    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, " << std::endl;
    std::cerr << "    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING," << std::endl;
    std::cerr << "    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;    " << std::endl;
    std::cerr << "    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER    " << std::endl;
    std::cerr << "    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT  " << std::endl;
    std::cerr << "    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN   " << std::endl;
    std::cerr << "    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE     " << std::endl;
    std::cerr << "    POSSIBILITY OF SUCH DAMAGE.                                         " << std::endl;
    
}

InstancePtr Application::getInputInstance() const {
    return inputInstance;
}

const std::vector<int>& Application::getVertexOrdering() const {
    return vertexOrdering;
}

HTDDecompositionPtr Application::getDecomposition() const {
    return decomposition;
}

options::OptionHandler& Application::getOptionHandler() {
    return opts;
}

options::Choice& Application::getHGInputParserChoice() {
    return optHGInputParser;
}

options::Choice& Application::getDecomposerChoice() {
    return optDecomposer;
}

options::Choice& Application::getPreprocessorChoice() {
    return optPreprocessor;
}

options::Choice& Application::getOrderingChoice() {
    return optOrdering;
}

options::Choice& Application::getSolverChoice() {
    return optSolver;
}

options::Choice& Application::getPrinterChoice() {
    return optPrinter;
}

const SolverFactory& Application::getSolverFactory() const {
    assert(solverFactory);
    return *solverFactory;
}

Printer& Application::getPrinter() const {
    assert(printer);
    return *printer;
}

void Application::setHGInputParser(HGInputParser& p) {
    hgInputParser = &p;
}

void Application::setDecomposer(Decomposer& d) {
    decomposer = &d;
}

void Application::setPreprocessor(Preprocessor& r) {
    preprocessor = &r;
}

Decomposer& Application::getDecomposer() const {
    assert(decomposer);
    return *decomposer;
}

void Application::setOrdering(Ordering& o) {
    ordering = &o;
}

void Application::setSolverFactory(SolverFactory& s) {
    solverFactory = &s;
}

void Application::setPrinter(Printer& p) {
    printer = &p;
}

bool Application::printInputInstance() const {
    return optPrintInputInstance.isUsed();
}

bool Application::printPreprocessedInstance() const {
    return optPrintPreprocessedInstance.isUsed();
}

bool Application::printDecomposition() const {
    return optPrintDecomposition.isUsed();
}

bool Application::printVertexOrdering() const {
    return optPrintVertexOrdering.isUsed();
}

bool Application::enumerate() const {
    return optEnumerate.isUsed();
}

int Application::enumerateLimit() const {
    return optEnumerate.getValue();
}

bool Application::modelCount() const {
    return optModelCount.isUsed();
}

BDDManager& Application::getBDDManager() const {
    return *bddManager;
}

ComputationManager& Application::getNSFManager() const {
    return *nsfManager;
}

htd::LibraryInstance* Application::getHTDManager() const {
    return htdManager;
}
