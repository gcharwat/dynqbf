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

#include <cassert>

#include "HTDTreeDecomposer.h"
#include "../Application.h"
#include "WidthFitnessFunction.h"
#include "HeightFitnessFunction.h"
#include "JoinNodeCountFitnessFunction.h"
#include "JoinNodeChildCountFitnessFunction.h"
#include "JoinNodeChildBagFitnessFunction.h"
#include "JoinNodeChildBagProductFitnessFunction.h"
#include "JoinNodeBagExponentialFitnessFunction.h"
#include "JoinNodeBagFitnessFunction.h"
#include "NSFSizeEstimationFitnessFunction.h"
#include "NSFSizeJoinEstimationFitnessFunction.h"
#include "InverseNSFSizeJoinEstimationFitnessFunction.h"
#include "VariableLevelFitnessFunction.h"
#include "RemovedLevelFitnessFunction.h"
#include "LeafNodeCountFitnessFunction.h"

#include <htd/main.hpp>


namespace decomposer {

    const std::string HTDTreeDecomposer::OPTION_SECTION = "Tree decomposition (-d td)";

    HTDTreeDecomposer::HTDTreeDecomposer(Application& app, bool newDefault)
    : Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
    , optGraphPreprocessing("td-preprocessing", "p", "Graph preprocessing before tree decomposition construction")
    , optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
    , optEliminationOrdering("elimination", "h", "Use heuristic <h> for bucket elimination")
//    , optJoinCompression("join-compression", "Enable subset-maximal compression at join nodes")
    , optNoEmptyRoot("no-empty-root", "Do not add an empty root to the tree decomposition")
    , optEmptyLeaves("empty-leaves", "Add empty leaves to the tree decomposition")
    , optPathDecomposition("path-decomposition", "Create a path decomposition")
    , optRootSelectionFitnessFunction("rs", "f", "Use fitness function <f> for decomposition root node selection")
    , optRootSelectionIterations("rsi", "i", "Randomly select <i> nodes as root, choose decomposition with best fitness value, 0 for #TD nodes", 1)
    , optDecompositionFitnessFunction("ds", "f", "Use fitness function <f> for decomposition selection")
    , optDecompositionIterations("dsi", "i", "Generate <i> tree decompositions, choose decomposition with best fitness value", 30)
    , optPrintStats("print-TD-stats", "Print detailed tree decomposition statistics") {
        optGraphPreprocessing.addCondition(selected);
        optGraphPreprocessing.addChoice("full", "all TD graph preprocessing optimizations enabled", true);
        optGraphPreprocessing.addChoice("advanced", "most TD graph preprocessing optimizations enabled");
        optGraphPreprocessing.addChoice("simple", "some TD graph preprocessing optimizations enabled");
        optGraphPreprocessing.addChoice("none", "TD graph preprocessing disabled");
        app.getOptionHandler().addOption(optGraphPreprocessing, OPTION_SECTION);

        optNormalization.addCondition(selected);
        optNormalization.addChoice("none", "no normalization", true);
        optNormalization.addChoice("weak", "weak normalization");
        optNormalization.addChoice("semi", "semi-normalization");
        optNormalization.addChoice("normalized", "normalization");
        app.getOptionHandler().addOption(optNormalization, OPTION_SECTION);

        optEliminationOrdering.addCondition(selected);
        optEliminationOrdering.addChoice("min-fill", "minimum fill ordering", true);
        optEliminationOrdering.addChoice("min-degree", "minimum degree ordering");
        optEliminationOrdering.addChoice("mcs", "maximum cardinality search");
        optEliminationOrdering.addChoice("mcsm", "enhanced maximum cardinality search");
        optEliminationOrdering.addChoice("natural", "natural order search");
        app.getOptionHandler().addOption(optEliminationOrdering, OPTION_SECTION);

//        optJoinCompression.addCondition(selected);
//        app.getOptionHandler().addOption(optJoinCompression, OPTION_SECTION);

        optNoEmptyRoot.addCondition(selected);
        app.getOptionHandler().addOption(optNoEmptyRoot, OPTION_SECTION);

        optEmptyLeaves.addCondition(selected);
        app.getOptionHandler().addOption(optEmptyLeaves, OPTION_SECTION);

        optPathDecomposition.addCondition(selected);
        app.getOptionHandler().addOption(optPathDecomposition, OPTION_SECTION);

        optRootSelectionFitnessFunction.addCondition(selected);
        optRootSelectionFitnessFunction.addChoice("none", "do not optimize selected root", true);
        optRootSelectionFitnessFunction.addChoice("height", "minimize decomposition height");
        optRootSelectionFitnessFunction.addChoice("est-join-effort", "minimize the sum over products of join node children bag sizes");
        optRootSelectionFitnessFunction.addChoice("removal-impact", "minimize the estimated total size of computed NSFs");
        optRootSelectionFitnessFunction.addChoice("removal-join-min", "minimize the estimated total size of computed NSFs in join nodes");
        optRootSelectionFitnessFunction.addChoice("removal-join-max", "maximize the estimated total size of computed NSFs in join nodes");
        optRootSelectionFitnessFunction.addChoice("variable-position", "prefer innermost variables to be removed first (relative location in TD and removal)");
        optRootSelectionFitnessFunction.addChoice("removed-level", "prefer innermost variables to be removed first (punish removal of variables with lower level)");
        app.getOptionHandler().addOption(optRootSelectionFitnessFunction, OPTION_SECTION);

        optRootSelectionIterations.addCondition(selected);
        app.getOptionHandler().addOption(optRootSelectionIterations, OPTION_SECTION);

        optDecompositionFitnessFunction.addCondition(selected);
        optDecompositionFitnessFunction.addChoice("none", "do not optimize decomposition");
        optDecompositionFitnessFunction.addChoice("dynamic", "for 2-QBFs: join-child-count, removed-level otherwise", true);
        optDecompositionFitnessFunction.addChoice("width", "minimize decomposition width");
        optDecompositionFitnessFunction.addChoice("join-count", "minimize number of join nodes");
        optDecompositionFitnessFunction.addChoice("join-bag-size", "minimize the sum over join node bag sizes");
        optDecompositionFitnessFunction.addChoice("join-child-count", "minimize number of join node children");
        optDecompositionFitnessFunction.addChoice("join-bag-size-exp", "minimize the sum over join node bag sizes to the power of number of children");
        optDecompositionFitnessFunction.addChoice("join-child-bag-size", "minimize the sum over join node children bag sizes");
        optDecompositionFitnessFunction.addChoice("est-join-effort", "minimize the sum over products of join node children bag sizes");
        optDecompositionFitnessFunction.addChoice("removal-impact", "minimize the estimated total size of computed NSFs");
        optDecompositionFitnessFunction.addChoice("removal-join-min", "minimize the estimated total size of computed NSFs in join nodes");
        optDecompositionFitnessFunction.addChoice("removal-join-max", "maximize the estimated total size of computed NSFs in join nodes");
        optDecompositionFitnessFunction.addChoice("variable-position", "prefer innermost variables to be removed first (relative location in TD and removal)");
        optDecompositionFitnessFunction.addChoice("removed-level", "prefer innermost variables to be removed first (punish removal of variables with lower level)");
        optDecompositionFitnessFunction.addChoice("leaf-count", "minimize the number of leaf nodes");

        app.getOptionHandler().addOption(optDecompositionFitnessFunction, OPTION_SECTION);

        optDecompositionIterations.addCondition(selected);
        app.getOptionHandler().addOption(optDecompositionIterations, OPTION_SECTION);

        optPrintStats.addCondition(selected);
        app.getOptionHandler().addOption(optPrintStats, OPTION_SECTION);

    }

    HTDDecompositionPtr HTDTreeDecomposer::decompose(const InstancePtr& instance) const {
        if (instance->hypergraph->vertexCount() == 0)
            throw std::runtime_error("Empty input instance.");

        htd::IOrderingAlgorithm * orderingAlgorithm;

        if (optEliminationOrdering.getValue() == "min-fill")
            orderingAlgorithm = new htd::MinFillOrderingAlgorithm(app.getHTDManager());
        else if (optEliminationOrdering.getValue() == "min-degree")
            orderingAlgorithm = new htd::MinDegreeOrderingAlgorithm(app.getHTDManager());
        else if (optEliminationOrdering.getValue() == "mcs")
            orderingAlgorithm = new htd::MaximumCardinalitySearchOrderingAlgorithm(app.getHTDManager());
        else if (optEliminationOrdering.getValue() == "mcsm")
            orderingAlgorithm = new htd::EnhancedMaximumCardinalitySearchOrderingAlgorithm(app.getHTDManager());
        else {
            assert(optEliminationOrdering.getValue() == "natural");
            orderingAlgorithm = new htd::NaturalOrderingAlgorithm(app.getHTDManager());
        }
        app.getHTDManager()->orderingAlgorithmFactory().setConstructionTemplate(orderingAlgorithm);

//        htd::BucketEliminationTreeDecompositionAlgorithm * treeDecompositionAlgorithm = new htd::BucketEliminationTreeDecompositionAlgorithm(app.getHTDManager());

        // TODO!!
        htd::ITreeDecompositionAlgorithm * treeDecompositionAlgorithm = NULL;
        if (true) {
            treeDecompositionAlgorithm = new htd::BucketEliminationTreeDecompositionAlgorithm(app.getHTDManager());
        } else {
            // experimental (disabled for now)
            treeDecompositionAlgorithm = new htd::SeparatorBasedTreeDecompositionAlgorithm(app.getHTDManager());
        }
        
//        // disable compression for join nodes
//        if (!(optJoinCompression.isUsed())) {
//            treeDecompositionAlgorithm->setCompressionEnabled(false); // disable compression
//            treeDecompositionAlgorithm->addManipulationOperation(new htd::CompressionOperation(app.getHTDManager(), false)); // compress everything, except join nodes
//        }
        
        app.getHTDManager()->treeDecompositionAlgorithmFactory().setConstructionTemplate(treeDecompositionAlgorithm);

        // set cover oder exact...
        // htd::SetCoverAlgorithmFactory::instance().setConstructionTemplate(new htd::HeuristicSetCoverAlgorithm());

        bool emptyLeaves = optEmptyLeaves.isUsed();
        bool emptyRoot = !optNoEmptyRoot.isUsed();

        htd::TreeDecompositionOptimizationOperation * operation;
        if (optRootSelectionFitnessFunction.getValue() == "height") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new HeightFitnessFunction());
        } else if (optRootSelectionFitnessFunction.getValue() == "est-join-effort") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new JoinNodeChildBagProductFitnessFunction());
        } else if (optRootSelectionFitnessFunction.getValue() == "removal-impact") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new NSFSizeEstimationFitnessFunction());
        } else if (optRootSelectionFitnessFunction.getValue() == "removal-join-min") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new NSFSizeJoinEstimationFitnessFunction());
        } else if (optRootSelectionFitnessFunction.getValue() == "removal-join-max") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new InverseNSFSizeJoinEstimationFitnessFunction());
        } else if (optRootSelectionFitnessFunction.getValue() == "variable-position") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new VariableLevelFitnessFunction(app));
        } else if (optRootSelectionFitnessFunction.getValue() == "removed-level") {
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), new RemovedLevelFitnessFunction(app));
        } else {
            assert(optRootSelectionFitnessFunction.getValue() == "none");
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager());
        }

        operation->setVertexSelectionStrategy(new htd::RandomVertexSelectionStrategy(optRootSelectionIterations.getValue() - 1));

        // Path decomposition
        if (optPathDecomposition.isUsed()) {
            operation->addManipulationOperation(new htd::JoinNodeReplacementOperation(app.getHTDManager()));
        }

        // Normalize
        if (optNormalization.getValue() == "none") {
            if (emptyRoot) operation->addManipulationOperation(new htd::AddEmptyRootOperation(app.getHTDManager()));
            if (emptyLeaves) operation->addManipulationOperation(new htd::AddEmptyLeavesOperation(app.getHTDManager()));
        } else if (optNormalization.getValue() == "weak") {
            operation->addManipulationOperation(new htd::WeakNormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false));
        } else if (optNormalization.getValue() == "semi") {
            operation->addManipulationOperation(new htd::SemiNormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false));
        } else if (optNormalization.getValue() == "normalized") {
            operation->addManipulationOperation(new htd::NormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false, false));
        } else {
            /* nothing to do */
        }

        htd::ITreeDecompositionAlgorithm* algorithm = app.getHTDManager()->treeDecompositionAlgorithmFactory().createInstance();
        algorithm->addManipulationOperation(operation);

        if (optDecompositionFitnessFunction.getValue() != "none") {
            htd::IterativeImprovementTreeDecompositionAlgorithm* iterativeAlgorithm;
            if (optDecompositionFitnessFunction.getValue() == "dynamic") {
                if (instance->quantifierCount() <= 2) {
                    iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeChildCountFitnessFunction());
                } else {
                    iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new RemovedLevelFitnessFunction(app));
                }
            }
            else if (optDecompositionFitnessFunction.getValue() == "width") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new WidthFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "join-count") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeCountFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "join-bag-size") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeBagFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-count") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeChildCountFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "join-bag-size-exp") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeBagExponentialFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-bag-size") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeChildBagFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "est-join-effort") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new JoinNodeChildBagProductFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "removal-impact") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new NSFSizeEstimationFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "removal-join-min") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new NSFSizeJoinEstimationFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "removal-join-max") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new InverseNSFSizeJoinEstimationFitnessFunction());
            } else if (optDecompositionFitnessFunction.getValue() == "variable-position") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new VariableLevelFitnessFunction(app));
            } else if (optDecompositionFitnessFunction.getValue() == "removed-level") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new RemovedLevelFitnessFunction(app));
            } else if (optDecompositionFitnessFunction.getValue() == "leaf-count") {
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, new LeafNodeCountFitnessFunction());
            } else {
                throw std::runtime_error("Invalid option");
            }

            iterativeAlgorithm->setIterationCount(optDecompositionIterations.getValue());
            iterativeAlgorithm->setNonImprovementLimit(-1);
            algorithm = iterativeAlgorithm;
        }


        htd::GraphPreprocessor * preprocessor = new htd::GraphPreprocessor(app.getHTDManager());

        if (std::string(optGraphPreprocessing.getValue()) == "none") {
            preprocessor->setPreprocessingStrategy(0);
        } else if (std::string(optGraphPreprocessing.getValue()) == "simple") {
            preprocessor->setPreprocessingStrategy(1);
        } else if (std::string(optGraphPreprocessing.getValue()) == "advanced") {
            preprocessor->setPreprocessingStrategy(2);
        } else if (std::string(optGraphPreprocessing.getValue()) == "full") {
            preprocessor->setPreprocessingStrategy(3);
        }

        htd::IPreprocessedGraph * preprocessedGraph = preprocessor->prepare(instance->hypergraph->internalGraph());

        htd::ITreeDecomposition* decomp = algorithm->computeDecomposition(instance->hypergraph->internalGraph(), *preprocessedGraph);
        delete preprocessedGraph;
        delete preprocessor;

        htd::IMutableTreeDecomposition* decompMutable = &(app.getHTDManager()->treeDecompositionFactory().accessMutableInstance(*decomp));
        HTDDecompositionPtr decomposition(decompMutable);

        if (optPrintStats.isUsed()) {
            printStatistics((instance->hypergraph->internalGraph()), *decomp);
        }

        return decomposition;
    }

    void HTDTreeDecomposer::printStatistics(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const {

        std::cout << "TD (nodes): " << decomposition.vertexCount() << std::endl;
        std::cout << "TD (leaf nodes): " << decomposition.leafCount() << std::endl;
        std::cout << "TD (join nodes): " << decomposition.joinNodeCount() << std::endl;
        std::cout << "TD (width): " << (decomposition.maximumBagSize() - 1) << std::endl;

        htd::FitnessEvaluation * eval;

        HeightFitnessFunction hff;
        eval = hff.fitness(graph, decomposition);
        std::cout << "TD (height): " << (int) (eval->at(0) * -1) << std::endl;
        delete eval;
        JoinNodeCountFitnessFunction jncff;
        eval = jncff.fitness(graph, decomposition);
        std::cout << "TD (join-count): " << (int) (eval->at(0) * -1) << std::endl;
        delete eval;
        JoinNodeBagFitnessFunction jnbff;
        eval = jnbff.fitness(graph, decomposition);
        std::cout << "TD (join-bag-size): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;
        JoinNodeChildCountFitnessFunction jnccff;
        eval = jnccff.fitness(graph, decomposition);
        std::cout << "TD (join-child-count): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;
        JoinNodeBagExponentialFitnessFunction jnbeff;
        eval = jnbeff.fitness(graph, decomposition);
        std::cout << "TD (join-bag-size-exp): " << (eval->at(0) * -1) << std::endl;
        delete eval;
        JoinNodeChildBagFitnessFunction jncbff;
        eval = jncbff.fitness(graph, decomposition);
        std::cout << "TD (join-child-bag-size): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;

        JoinNodeChildBagProductFitnessFunction jncbpff;
        eval = jncbpff.fitness(graph, decomposition);
        std::cout << "TD (est-join-effort): " << (eval->at(0) * -1) << std::endl;
        delete eval;
        NSFSizeEstimationFitnessFunction nseff;
        eval = nseff.fitness(graph, decomposition);
        std::cout << "TD (removal-impact): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;
        NSFSizeJoinEstimationFitnessFunction nsjeff;
        eval = nsjeff.fitness(graph, decomposition);
        std::cout << "TD (removal-join-min): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;
        InverseNSFSizeJoinEstimationFitnessFunction insjeff;
        eval = insjeff.fitness(graph, decomposition);
        std::cout << "TD (removal-join-max): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;

        VariableLevelFitnessFunction vlff(app);
        eval = vlff.fitness(graph, decomposition);
        std::cout << "TD (variable-position): " << (eval->at(0) * -1) << std::endl;
        delete eval;
        RemovedLevelFitnessFunction rlff(app);
        eval = rlff.fitness(graph, decomposition);
        std::cout << "TD (removed-level): " << (long) (eval->at(0) * -1) << std::endl;
        delete eval;
    }

} // namespace decomposer
