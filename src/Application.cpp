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

#include <ctime>
#include <iostream>
#include <sstream>
#include <cassert>

#include "Utils.h"
#include "DynQBFConfig.h"
#include "Application.h"
#include "BDDManager.h"
#include "HeuristicNSFManager.h"
#include "Computation.h"
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

#include "decomposer/Dummy.h"
#include "decomposer/HTDTreeDecomposer.h"

#include "ordering/LexicographicalOrdering.h"
#include "ordering/InstanceOrdering.h"
#include "ordering/LevelOrdering.h"
#include "ordering/LevelInverseOrdering.h"
#include "ordering/MinCutOrdering.h"
#include "ordering/MaxBagOrdering.h"
#include "ordering/MaxClauseOrdering.h"
#include "ordering/MinDegreeOrdering.h"

#include "solver/dummy/SolverFactory.h"
#include "solver/bdd/qsat/QSatCNFSolverFactory.h"
#include "solver/bdd/qsat/QSat2CNFSolverFactory.h"
#include "solver/bdd/qsat/QSatBDDSolverFactory.h"

#include "printer/Quiet.h"
#include "printer/Progress.h"
#include "printer/Debug.h"
#include "printer/Performance.h"
#include "solver/bdd/qsat/QSat2CNFSolverFactory.h"

const std::string Application::MODULE_SECTION = "Module selection";

Application::Application(const std::string& binaryName)
: binaryName(binaryName)
, optHelp("h", "Print usage information and exit")
, optHGInputParser("i", "input-format", "Specify the instance input format")
, optDecomposer("d", "decomposer", "Use decomposition method <decomposer>")
, optPreprocessor("r", "preprocessor", "Use instance preprocessor <preprocessor>")
, optOrdering("o", "ordering", "Use BDD variable ordering <ordering>")
, optSolver("p", "problem-solver", "Use <problem-solver> to solve problem")
, optPrinter("output", "module", "Print information during the run using <module>")
, optPrintInputInstance("print-instance", "Print the input hypergraph")
, optPrintPreprocessedInstance("print-preprocessed", "Print the preprocessed hypergraph")
, optPrintDecomposition("print-decomposition", "Print the computed decomposition")
, optPrintVertexOrdering("print-ordering", "Print the computed vertex order")
, optOnlyParseInstance("only-parse-instance", "Only construct hypergraph and exit")
, optOnlyDecomposeInstance("only-decompose", "Only parse input instance, decompose it and exit")
, optEnumerate("enumerate", "Enumerate models (for outermost existential quantifier, satisfiable instances)")
, optSeed("seed", "n", "Initialize random number generator with seed <n>")
//, decomposer(0)
//, solverFactory(0) 
{
}

int Application::run(int argc, char** argv) {
    
    opts.addOption(optHelp);
    options::HelpObserver helpObserver(*this, optHelp);
    opts.registerObserver(helpObserver);

    opts.addOption(optOnlyParseInstance);
    opts.addOption(optOnlyDecomposeInstance);
    opts.addOption(optPrintInputInstance);
    opts.addOption(optPrintPreprocessedInstance);
    opts.addOption(optPrintDecomposition);
    opts.addOption(optPrintVertexOrdering);
    opts.addOption(optEnumerate);
    opts.addOption(optSeed);

    opts.addOption(optHGInputParser, MODULE_SECTION); // uncomment to add to selection
    parser::DIMACSDriver dimacsParser(*this, true);
    //parser::DIMACSIncidenceDriver dimacsIncidenceParser(*this);

    opts.addOption(optPreprocessor, MODULE_SECTION);
    preprocessor::NoPreprocessor noPreprocessor(*this, true);
//    preprocessor::UnitLiteralPreprocessor unitLiteralPreprocessor(*this);
//    preprocessor::CNF3Preprocessor cnf3Preprocessor(*this);
//    preprocessor::SplitPreprocessor splitPreprocessor(*this);
    preprocessor::CombinedPreprocessor combinedPreprocessor(*this);
    

    opts.addOption(optSolver, MODULE_SECTION);
    solver::bdd::qsat::QSatCNFSolverFactory qsatSolverCNFFactory(*this, true);
    solver::bdd::qsat::QSat2CNFSolverFactory qsat2SolverCNFFactory(*this);
    //    solver::bdd::qsat::QSatDNFSolverFactory qsatSolverDNFFactory(*this);
    solver::bdd::qsat::QSatBDDSolverFactory qsatSolverBDDFactory(*this);
    solver::dummy::SolverFactory dummySolverFactory(*this);

    opts.addOption(optDecomposer, MODULE_SECTION);
    decomposer::HTDTreeDecomposer treeDecomposer(*this, true);
    decomposer::Dummy dummyDecomposer(*this);

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
    printer::Performance performancePrinter(*this);
    //printer::Visualization visualizationPrinter(*this);

    bddManager = new BDDManager(*this);
    nsfManager = new HeuristicNSFManager(*this);
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
    srand(seed);
    
    RESULT result = RESULT::UNDECIDED;

    try {
        // Parse instance
        inputInstance = hgInputParser->parse(std::cin);
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
        printer->afterComputation();

        // Return result
        result = solver->decide(*computation);
        if (enumerate()) {
            BDD answer = solver->solutions(*computation);
            printer->models(answer, solverFactory->getVariables());
        }
        
        delete computation;

    } catch (AbortException e) {
        result = e.getResult();
        std::cout << "Notice: " << e.what() << std::endl;
    }

    delete bddManager;
    delete nsfManager;

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

    printer->result(result);

    decomposition.reset();
    inputInstance.reset();

    return exitCode;
}

void Application::usage() const {
    std::cerr << "Version " << DYNQBF_VERSION_MAJOR << "." << DYNQBF_VERSION_MINOR << " (Built on " __DATE__ << " at " << __TIME__ << ")" << std::endl;
    std::cerr << "Usage: " << binaryName << " [options] < instance" << std::endl;
    opts.printHelp();
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

BDDManager& Application::getBDDManager() const {
    return *bddManager;
}

BaseNSFManager& Application::getNSFManager() const {
    return *nsfManager;
}

htd::LibraryInstance* Application::getHTDManager() const {
    return htdManager;
}
