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

#include <htd/main.hpp>


namespace decomposer {

    const std::string HTDTreeDecomposer::OPTION_SECTION = "Tree decomposition (-d td)";

    HTDTreeDecomposer::HTDTreeDecomposer(Application& app, bool newDefault)
    : Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
    , optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
    , optEliminationOrdering("elimination", "h", "Use heuristic <h> for bucket elimination")
    , optJoinCompression("join-compression", "Enable subset-maximal compression at join nodes")
    , optNoEmptyRoot("no-empty-root", "Do not add an empty root to the tree decomposition")
    , optEmptyLeaves("empty-leaves", "Add empty leaves to the tree decomposition")
    , optPathDecomposition("path-decomposition", "Create a path decomposition")
    , optRootSelectionFitnessFunction("rs", "f", "Use fitness function <f> for decomposition root node selection")
    , optRootSelectionIterations("rsi", "i", "Randomly select <i> nodes as root, choose decomposition with best fitness value, 0 for #td nodes", 1)
    , optDecompositionFitnessFunction("ds", "f", "Use fitness function <f> for decomposition selection")
    , optDecompositionIterations("dsi", "i", "Generate <i> tree decompositions, choose decomposition with best fitness value", 10) {
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
        optEliminationOrdering.addChoice("natural", "natural order search");
        app.getOptionHandler().addOption(optEliminationOrdering, OPTION_SECTION);

        optJoinCompression.addCondition(selected);
        app.getOptionHandler().addOption(optJoinCompression, OPTION_SECTION);
        
        optNoEmptyRoot.addCondition(selected);
        app.getOptionHandler().addOption(optNoEmptyRoot, OPTION_SECTION);

        optEmptyLeaves.addCondition(selected);
        app.getOptionHandler().addOption(optEmptyLeaves, OPTION_SECTION);

        optPathDecomposition.addCondition(selected);
        app.getOptionHandler().addOption(optPathDecomposition, OPTION_SECTION);

        optRootSelectionFitnessFunction.addCondition(selected);
        optRootSelectionFitnessFunction.addChoice("none", "do not optimize selected root", true);
        optRootSelectionFitnessFunction.addChoice("height", "minimize decomposition height");
        optRootSelectionFitnessFunction.addChoice("variable-level", "prefer innermost variables to be removed first (relative location in td and removal)");
        optRootSelectionFitnessFunction.addChoice("removed-level", "prefer innermost variables to be removed first (punish removal of variables with lower level)");
        optRootSelectionFitnessFunction.addChoice("join-child-bag-prod", "minimize the sum over products of join node children bag sizes");
        optRootSelectionFitnessFunction.addChoice("nsf", "minimize the estimated total size of computed NSFs");
        optRootSelectionFitnessFunction.addChoice("join-nsf", "minimize the estimated total size of computed NSFs in join nodes");
        optRootSelectionFitnessFunction.addChoice("join-nsf-i", "maximize the estimated total size of computed NSFs in join nodes");
        app.getOptionHandler().addOption(optRootSelectionFitnessFunction, OPTION_SECTION);

        optRootSelectionIterations.addCondition(selected);
        app.getOptionHandler().addOption(optRootSelectionIterations, OPTION_SECTION);

        optDecompositionFitnessFunction.addCondition(selected);
        optDecompositionFitnessFunction.addChoice("none", "do not optimize decomposition");
        optDecompositionFitnessFunction.addChoice("width", "minimize decomposition width");
        optDecompositionFitnessFunction.addChoice("join-count", "minimize number of join nodes");
        optDecompositionFitnessFunction.addChoice("join-bag", "minimize the sum over join node bag sizes");
        optDecompositionFitnessFunction.addChoice("join-child-count", "minimize number of join node children");
        optDecompositionFitnessFunction.addChoice("join-child-bag", "minimize the sum over join node children bag sizes");
        optDecompositionFitnessFunction.addChoice("join-child-bag-prod", "minimize the sum over products of join node children bag sizes", true);
        optDecompositionFitnessFunction.addChoice("join-bag-exp", "minimize the sum over join node bag sizes to the power of number of children");
        optDecompositionFitnessFunction.addChoice("nsf", "minimize the estimated total size of computed NSFs");
        optDecompositionFitnessFunction.addChoice("join-nsf", "minimize the estimated total size of computed NSFs in join nodes");
        optDecompositionFitnessFunction.addChoice("join-nsf-i", "maximize the estimated total size of computed NSFs in join nodes");
        
        app.getOptionHandler().addOption(optDecompositionFitnessFunction, OPTION_SECTION);
        
        optDecompositionIterations.addCondition(selected);
        app.getOptionHandler().addOption(optDecompositionIterations, OPTION_SECTION);

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
        else {
            assert(optEliminationOrdering.getValue() == "natural");
            orderingAlgorithm = new htd::NaturalOrderingAlgorithm(app.getHTDManager());
        }
        app.getHTDManager()->orderingAlgorithmFactory().setConstructionTemplate(orderingAlgorithm);
        
        htd::BucketEliminationTreeDecompositionAlgorithm * treeDecompositionAlgorithm = new htd::BucketEliminationTreeDecompositionAlgorithm(app.getHTDManager());
        
        // disable compression for join nodes
        if (!(optJoinCompression.isUsed())) {
            treeDecompositionAlgorithm->setCompressionEnabled(false);
            treeDecompositionAlgorithm->addManipulationOperation(new htd::CompressionOperation(app.getHTDManager(), false));
        }
        app.getHTDManager()->treeDecompositionAlgorithmFactory().setConstructionTemplate(treeDecompositionAlgorithm); 
        
        // set cover oder exact...
        // htd::SetCoverAlgorithmFactory::instance().setConstructionTemplate(new htd::HeuristicSetCoverAlgorithm());

        bool emptyLeaves = optEmptyLeaves.isUsed();
        bool emptyRoot = !optNoEmptyRoot.isUsed();

        htd::TreeDecompositionOptimizationOperation * operation;
        if (optRootSelectionFitnessFunction.getValue() == "height") {
            HeightFitnessFunction heightFitnessFunction;
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), heightFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "variable-level") {
            VariableLevelFitnessFunction variableLevelFitnessFunction(app);
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), variableLevelFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "removed-level") {
            RemovedLevelFitnessFunction removedLevelFitnessFunction(app);
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), removedLevelFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "join-child-bag-prod") {
            JoinNodeChildBagProductFitnessFunction joinNodeChildBagProductFitnessFunction;
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), joinNodeChildBagProductFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "nsf") {
            NSFSizeEstimationFitnessFunction nsfSizeEstimationFitnessFunction;
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), nsfSizeEstimationFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "join-nsf") {
            NSFSizeJoinEstimationFitnessFunction nsfSizeJoinEstimationFitnessFunction;
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), nsfSizeJoinEstimationFitnessFunction);
        } else if (optRootSelectionFitnessFunction.getValue() == "join-nsf-i") {
            InverseNSFSizeJoinEstimationFitnessFunction inverseNSFSizeJoinEstimationFitnessFunction;
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager(), inverseNSFSizeJoinEstimationFitnessFunction);
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

        if (optDecompositionFitnessFunction.isUsed() && optDecompositionFitnessFunction.getValue() != "none") {
            htd::IterativeImprovementTreeDecompositionAlgorithm* iterativeAlgorithm;
            if (optDecompositionFitnessFunction.getValue() == "width") {
                WidthFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-count") {
                JoinNodeCountFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            }else if (optDecompositionFitnessFunction.getValue() == "join-bag") {
                JoinNodeBagFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-count") {
                JoinNodeChildCountFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-bag") {
                JoinNodeChildBagFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-bag-prod") {
                JoinNodeChildBagProductFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-bag-exp") {
                JoinNodeBagExponentialFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "nsf") {
                NSFSizeEstimationFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-nsf") {
                NSFSizeJoinEstimationFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-nsf-i") {
                InverseNSFSizeJoinEstimationFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else {
                throw std::runtime_error("Invalid option");
            }
            iterativeAlgorithm->setIterationCount(optDecompositionIterations.getValue());
            iterativeAlgorithm->setNonImprovementLimit(-1);
            algorithm = iterativeAlgorithm;
        }

        htd::ITreeDecomposition* decomp = algorithm->computeDecomposition(instance->hypergraph->internalGraph());

        htd::IMutableTreeDecomposition* decompMutable = &(app.getHTDManager()->treeDecompositionFactory().accessMutableInstance(*decomp));
        HTDDecompositionPtr decomposition(decompMutable);
        return decomposition;
    }

} // namespace decomposer
