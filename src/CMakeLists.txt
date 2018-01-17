cmake_minimum_required (VERSION 2.8)

project (DYNQBF)
        
# Set project version numbers
set (DYNQBF_VERSION_MAJOR 1)
set (DYNQBF_VERSION_MINOR 1)
set (DYNQBF_VERSION_PATCH 1)
set (DYNQBF_VERSION_PRERELEASE "")
        
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} update-index --refresh WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}" RESULT_VARIABLE DYNQBF_GIT_RETURN_VALUE OUTPUT_VARIABLE DYNQBF_GIT_CURRENT_COMMIT_ID)
    if(NOT ${DYNQBF_GIT_RETURN_VALUE} EQUAL 0)
        set(DYNQBF_GIT_COMMIT_ID "")
        message(WARNING "Could not derive Git commit ID. Build will not contain Git revision info.")
    endif()
    string(REPLACE "\n" "" DYNQBF_GIT_COMMIT_ID "${DYNQBF_GIT_CURRENT_COMMIT_ID}")
    
    execute_process(COMMAND ${GIT_EXECUTABLE} update-index --refresh WORKING_DIRECTORY "${htd_dir}")
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always WORKING_DIRECTORY "${htd_dir}" RESULT_VARIABLE HTD_GIT_RETURN_VALUE OUTPUT_VARIABLE HTD_GIT_CURRENT_COMMIT_ID)
    if(NOT ${HTD_GIT_RETURN_VALUE} EQUAL 0)
        set(HTD_GIT_COMMIT_ID "")
        message(WARNING "Could not derive Git commit ID. Build will not contain Git revision info.")
    endif()
    string(REPLACE "\n" "" HTD_GIT_COMMIT_ID "${HTD_GIT_CURRENT_COMMIT_ID}")
else()
    set(DYNQBF_GIT_COMMIT_ID "")
    set(HTD_GIT_COMMIT_ID "")
    message(WARNING "Could not find Git installation. Build will not contain Git revision info.")
endif()

# Configure the header file to pass some of the CMake settings to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/DynQBFConfig.h.in"
  "${PROJECT_BINARY_DIR}/DynQBFConfig.h"
)

# Compile using C++11 standard
add_definitions("-Wall -pedantic -std=gnu++11") 

set(CMAKE_CXX_FLAGS_GPROF "-O3 -g -pg")
set(CMAKE_CXX_FLAGS_PROFILER "-O3 -g")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBRARIES OFF)

if(NOT APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Add additional directories for library search
include_directories("/opt/local/include")

# Add the binary tree to the search path for include files
include_directories("${PROJECT_BINARY_DIR}")

# Include the library header files
include_directories(${cudd_dir} ${cudd_dir}/cudd ${cudd_dir}/cplusplus ${cudd_dir}/st ${cudd_dir}/mtr ${cudd_dir}/epd ${cudd_dir}/util ${htd_dir}/include)

# Add required libraries for linking
add_library(htd STATIC IMPORTED)
set_target_properties(htd PROPERTIES IMPORTED_LOCATION ${htd_lib})
add_library(cudd STATIC IMPORTED)
set_target_properties(cudd PROPERTIES IMPORTED_LOCATION ${cudd_lib})

# Optionally compile with DepQBF support
if(${depqbf_enabled})
    add_definitions(-DDEPQBF_ENABLED=${depqbf_enabled})
    include_directories(${depqbf_dir})
    add_library(qdpll STATIC IMPORTED)
    set_target_properties(qdpll PROPERTIES IMPORTED_LOCATION ${depqbf_lib})
    set(dynqbf-sources 
        nsf/StandardDependencyCacheComputation.cpp
    )
endif()

set(dynqbf-sources ${dynqbf-sources}
    Utils.cpp
    Application.cpp
    options/Option.cpp
    options/ValueOption.cpp
    options/SingleValueOption.cpp
    options/MultiValueOption.cpp
    options/DefaultIntegerValueOption.cpp
    options/Choice.cpp
    options/OptionHandler.cpp
    options/HelpObserver.cpp
    Instance.cpp
    Module.cpp
    BDDManager.cpp
    nsf/NSF.cpp
    nsf/ComputationManager.cpp
    nsf/Computation.cpp
    nsf/CacheComputation.cpp
    nsf/SimpleDependencyCacheComputation.cpp    
    Variable.cpp
    HGInputParser.cpp
    parser/DIMACSDriver.cpp
    Preprocessor.cpp
    preprocessor/NoPreprocessor.cpp
    preprocessor/UnitLiteralPreprocessor.cpp
    preprocessor/CNF3Preprocessor.cpp
    preprocessor/SplitPreprocessor.cpp
    preprocessor/CombinedPreprocessor.cpp
    Decomposer.cpp
    decomposer/HTDTreeDecomposer.cpp
    decomposer/SingleNodeDecomposer.cpp
    decomposer/HeightFitnessFunction.cpp
    decomposer/JoinNodeChildCountFitnessFunction.cpp
    decomposer/JoinNodeChildBagFitnessFunction.cpp
    decomposer/JoinNodeChildBagProductFitnessFunction.cpp
    decomposer/JoinNodeBagExponentialFitnessFunction.cpp
    decomposer/JoinNodeBagFitnessFunction.cpp
    decomposer/JoinNodeCountFitnessFunction.cpp
    decomposer/WidthFitnessFunction.cpp
    decomposer/VariableLevelFitnessFunction.cpp
    decomposer/RemovedLevelFitnessFunction.cpp
    decomposer/NSFSizeEstimationFitnessFunction.cpp
    decomposer/NSFSizeJoinEstimationFitnessFunction.cpp
    decomposer/InverseNSFSizeJoinEstimationFitnessFunction.cpp
    decomposer/LeafNodeCountFitnessFunction.cpp
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
    solver/bdd/qsat/QSatCNFEDMSolverFactory.cpp
    solver/bdd/qsat/QSatCNFEDMSolver.cpp
    solver/bdd/qsat/QSatCNFLDMSolverFactory.cpp
    solver/bdd/qsat/QSatCNFLDMSolver.cpp
    solver/bdd/qsat/QSat2CNFSolverFactory.cpp
    solver/bdd/qsat/QSat2CNFEDMSolver.cpp
    solver/bdd/qsat/QSatBDDSolverFactory.cpp
    solver/bdd/qsat/QSatBDDSolver.cpp
    Printer.cpp
    printer/Quiet.cpp
    printer/Progress.cpp
    printer/Debug.cpp
    printer/Verbose.cpp
    printer/Performance.cpp
)
            
add_library(dynqbf-objects ${dynqbf-sources})

# add the executable
add_executable(dynqbf main.cpp)

# link the libraries
#set(dynqbf-linking dynqbf-objects cudd htd ${dynqbf-linking})

if(${depqbf_enabled})
    target_link_libraries(dynqbf dynqbf-objects cudd htd qdpll)
else()
    target_link_libraries(dynqbf dynqbf-objects cudd htd)
endif()
