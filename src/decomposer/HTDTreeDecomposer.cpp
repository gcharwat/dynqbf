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
#include "JoinNodeBagFitnessFunction.h"
#include "VariableLevelFitnessFunction.h"

#include <htd/main.hpp>


namespace decomposer {

    const std::string HTDTreeDecomposer::OPTION_SECTION = "Tree decomposition";

    HTDTreeDecomposer::HTDTreeDecomposer(Application& app, bool newDefault)
    : Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
    , optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
    , optEliminationOrdering("elimination", "h", "Use heuristic <h> for bucket elimination")
    , optNoEmptyRoot("no-empty-root", "Do not add an empty root to the tree decomposition")
    , optEmptyLeaves("empty-leaves", "Add empty leaves to the tree decomposition")
    , optPathDecomposition("path-decomposition", "Create a path decomposition")
    , optRootSelectionFitnessFunction("root-strategy", "f", "Use fitness function <f> for decomposition root node selection")
    , optRootSelectionIterations("root-strategy-iterations", "i", "Randomly select <i> nodes as root, choose decomposition with best fitness value", 10)
    , optDecompositionFitnessFunction("decomposition-strategy", "f", "Use fitness function <f> for decomposition selection")
    , optDecompositionIterations("decomposition-strategy-iterations", "i", "Generate <i> tree decompositions, choose decomposition with best fitness value", 10) {
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

        optNoEmptyRoot.addCondition(selected);
        app.getOptionHandler().addOption(optNoEmptyRoot, OPTION_SECTION);

        optEmptyLeaves.addCondition(selected);
        app.getOptionHandler().addOption(optEmptyLeaves, OPTION_SECTION);

        optPathDecomposition.addCondition(selected);
        app.getOptionHandler().addOption(optPathDecomposition, OPTION_SECTION);

        optRootSelectionFitnessFunction.addCondition(selected);
        optRootSelectionFitnessFunction.addChoice("none", "do not optimize selected root", true);
        optRootSelectionFitnessFunction.addChoice("height", "minimize decomposition height");
        optRootSelectionFitnessFunction.addChoice("variable-level", "prefer innermost variables to be removed first");
        app.getOptionHandler().addOption(optRootSelectionFitnessFunction, OPTION_SECTION);

        optRootSelectionIterations.addCondition(selected);
        app.getOptionHandler().addOption(optRootSelectionIterations, OPTION_SECTION);

        optDecompositionFitnessFunction.addCondition(selected);
        optDecompositionFitnessFunction.addChoice("none", "do not optimize decomposition", true);
        optDecompositionFitnessFunction.addChoice("width", "minimize decomposition width");
        optDecompositionFitnessFunction.addChoice("join-count", "minimize number of join nodes");
        optDecompositionFitnessFunction.addChoice("join-bag-size", "minimize the sum over join node bag sizes");
        optDecompositionFitnessFunction.addChoice("join-child-count", "minimize number of join node children");
        optDecompositionFitnessFunction.addChoice("join-child-bag-size", "minimize the sum over join node children bag sizes");
        optDecompositionFitnessFunction.addChoice("join-child-bag-size-product", "minimize the sum over products of join node children bag sizes");
        
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
        app.getHTDManager()->treeDecompositionAlgorithmFactory().setConstructionTemplate(new htd::BucketEliminationTreeDecompositionAlgorithm(app.getHTDManager()));
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
        } else {
            assert(optRootSelectionFitnessFunction.getValue() == "none");
            operation = new htd::TreeDecompositionOptimizationOperation(app.getHTDManager());
        }

        operation->setVertexSelectionStrategy(new htd::RandomVertexSelectionStrategy(optRootSelectionIterations.getValue()));

        // Path decomposition
        if (optPathDecomposition.isUsed()) {
            operation->addManipulationOperations({new htd::JoinNodeReplacementOperation(app.getHTDManager())});
        }

        // Normalize
        if (optNormalization.getValue() == "none") {
            if (emptyRoot) operation->addManipulationOperations({new htd::AddEmptyRootOperation(app.getHTDManager())});
            if (emptyLeaves) operation->addManipulationOperations({new htd::AddEmptyLeavesOperation(app.getHTDManager())});
        } else if (optNormalization.getValue() == "weak") {
            operation->addManipulationOperations({new htd::WeakNormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false)});
        } else if (optNormalization.getValue() == "semi") {
            operation->addManipulationOperations({new htd::SemiNormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false)});
        } else if (optNormalization.getValue() == "normalized") {
            operation->addManipulationOperations({new htd::NormalizationOperation(app.getHTDManager(), emptyRoot, emptyLeaves, false, false)});
        } else {
            /* nothing to do */
        }

        htd::ITreeDecompositionAlgorithm * algorithm = app.getHTDManager()->treeDecompositionAlgorithmFactory().getTreeDecompositionAlgorithm();
        algorithm->addManipulationOperation(operation);

        if (optDecompositionFitnessFunction.isUsed() && optDecompositionFitnessFunction.getValue() != "none") {
            htd::IterativeImprovementTreeDecompositionAlgorithm * iterativeAlgorithm;
            if (optDecompositionFitnessFunction.getValue() == "width") {
                WidthFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-count") {
                JoinNodeCountFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            }else if (optDecompositionFitnessFunction.getValue() == "join-bag-size") {
                JoinNodeBagFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-count") {
                JoinNodeChildCountFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-bag-size") {
                JoinNodeChildBagFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else if (optDecompositionFitnessFunction.getValue() == "join-child-bag-size-product") {
                JoinNodeChildBagProductFitnessFunction fitnessFunction;
                iterativeAlgorithm = new htd::IterativeImprovementTreeDecompositionAlgorithm(app.getHTDManager(), algorithm, fitnessFunction);
            } else {
                throw std::runtime_error("Invalid option");
            }
            iterativeAlgorithm->setIterationCount(optDecompositionIterations.getValue());
            iterativeAlgorithm->setNonImprovementLimit(-1);
            algorithm = iterativeAlgorithm;
        }

        htd::ITreeDecomposition * decomp = algorithm->computeDecomposition(instance->hypergraph->internalGraph());

        htd::IMutableTreeDecomposition * decompMutable = &(app.getHTDManager()->treeDecompositionFactory().accessMutableTreeDecomposition(*decomp));
        HTDDecompositionPtr decomposition(decompMutable);
        return decomposition;
    }

} // namespace decomposer
