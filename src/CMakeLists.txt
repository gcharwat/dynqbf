cmake_minimum_required (VERSION 2.8)

project (DYNQBF)
        
# Set project version numbers
set (DYNQBF_VERSION_MAJOR 0)
set (DYNQBF_VERSION_MINOR 2)

# Configure the header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/DynQBFConfig.h.in"
  "${PROJECT_BINARY_DIR}/DynQBFConfig.h"
)

# Compile using C++11 standard
add_definitions("-Wall -pedantic -std=gnu++11") 

set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_CXX_FLAGS_PROFILER "-O3 -g")

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")

# Add additional directories for library search
include_directories("/opt/local/include")

# Add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")

# Include the cudd and htd header files
include_directories(${cudd_dir} ${cudd_dir}/cudd ${cudd_dir}/cplusplus ${cudd_dir}/st ${cudd_dir}/mtr ${cudd_dir}/epd ${cudd_dir}/util ${htd_dir}/include)

# Add required libraries for linking
add_library(htd STATIC IMPORTED)
set_target_properties(htd PROPERTIES IMPORTED_LOCATION ${htd_lib})
add_library(cudd STATIC IMPORTED)
set_target_properties(cudd PROPERTIES IMPORTED_LOCATION ${cudd_lib})

add_library(dynqbf-objects
    Utils.cpp
    Application.cpp
    options/Option.cpp
    options/ValueOption.cpp
    options/SingleValueOption.cpp
    options/MultiValueOption.cpp
    options/Choice.cpp
    options/OptionHandler.cpp
    options/HelpObserver.cpp
    Module.cpp
    BDDManager.cpp
    NSFManager.cpp
    Computation.cpp
    Variable.cpp
    HGInputParser.cpp
    parser/DIMACSDriver.cpp
    Preprocessor.cpp
    preprocessor/NoPreprocessor.cpp
    preprocessor/UnitLiteralPreprocessor.cpp
    Decomposer.cpp
    decomposer/Dummy.cpp
    decomposer/HTDTreeDecomposer.cpp
    Ordering.cpp
    ordering/InstanceOrdering.cpp
    ordering/LexicographicalOrdering.cpp
    ordering/LevelOrdering.cpp
    ordering/LevelInverseOrdering.cpp
    ordering/MinCutOrdering.cpp
    ordering/MaxBagOrdering.cpp
    ordering/MaxClauseOrdering.cpp
    ordering/MinDegreeOrdering.cpp
    Solver.cpp
    SolverFactory.cpp
    solver/dummy/DummySolver.cpp
    solver/dummy/SolverFactory.cpp
    solver/bdd/qsat/QSatCNFSolverFactory.cpp
    solver/bdd/qsat/QSatCNFEDMSolver.cpp
    solver/bdd/qsat/QSatCNFLDMSolver.cpp
    solver/bdd/qsat/QSatBDDSolverFactory.cpp
    solver/bdd/qsat/QSatBDDSolver.cpp
    Printer.cpp
    printer/Quiet.cpp
    printer/Progress.cpp
    printer/Debug.cpp
    printer/Performance.cpp
)

# add the executable
add_executable(dynqbf main.cpp)

# link the libraries 
target_link_libraries(dynqbf dynqbf-objects cudd htd)
    
