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

#include <htd/main.hpp>


namespace decomposer {

    const std::string HTDTreeDecomposer::OPTION_SECTION = "Tree decomposition";

    HTDTreeDecomposer::HTDTreeDecomposer(Application& app, bool newDefault)
    : Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
    , optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
    , optEliminationOrdering("elimination", "h", "Use heuristic <h> for bucket elimination")
    , optNoEmptyRoot("no-empty-root", "Do not add an empty root to the tree decomposition")
    , optEmptyLeaves("empty-leaves", "Add empty leaves to the tree decomposition")
    , optPathDecomposition("path-decomposition", "Create a path decomposition") {
        optNormalization.addCondition(selected);
        optNormalization.addChoice("weak", "weak normalization", true);
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
    }

    HTDDecompositionPtr HTDTreeDecomposer::decompose(const HTDHypergraphPtr& instance) const {
        if (instance->vertexCount() == 0)
            throw std::runtime_error("Empty input instance.");

        htd::IOrderingAlgorithm * orderingAlgorithm;

        if (optEliminationOrdering.getValue() == "min-fill")
            orderingAlgorithm = new htd::MinFillOrderingAlgorithm();
        else if (optEliminationOrdering.getValue() == "min-degree")
            orderingAlgorithm = new htd::MinDegreeOrderingAlgorithm();
        else if (optEliminationOrdering.getValue() == "mcs")
            orderingAlgorithm = new htd::MaximumCardinalitySearchOrderingAlgorithm();
        else {
            assert(optEliminationOrdering.getValue() == "natural");
            orderingAlgorithm = new htd::NaturalOrderingAlgorithm();
        }
        htd::OrderingAlgorithmFactory::instance().setConstructionTemplate(orderingAlgorithm);
        htd::TreeDecompositionAlgorithmFactory::instance().setConstructionTemplate(new htd::BucketEliminationTreeDecompositionAlgorithm());
        // set cover oder exact...
        // htd::SetCoverAlgorithmFactory::instance().setConstructionTemplate(new htd::HeuristicSetCoverAlgorithm());

        bool emptyLeaves = optEmptyLeaves.isUsed();
        bool emptyRoot = !optNoEmptyRoot.isUsed();

        // Path decomposition
        if (optPathDecomposition.isUsed()) {
            htd::TreeDecompositionAlgorithmFactory::instance().addManipulationOperations({new htd::JoinNodeReplacementOperation()});
        }

        // Normalize
        if (optNormalization.getValue() == "weak") {
            htd::TreeDecompositionAlgorithmFactory::instance().addManipulationOperations({new htd::WeakNormalizationOperation(emptyRoot, emptyLeaves, false)});
        } else if (optNormalization.getValue() == "semi") {
            htd::TreeDecompositionAlgorithmFactory::instance().addManipulationOperations({new htd::SemiNormalizationOperation(emptyRoot, emptyLeaves, false)});
        } else if (optNormalization.getValue() == "normalized") {
            htd::TreeDecompositionAlgorithmFactory::instance().addManipulationOperations({new htd::NormalizationOperation(emptyRoot, emptyLeaves, false, false)});
        } else {
            /* nothing to do */
        }

        htd::ITreeDecompositionAlgorithm * algorithm = htd::TreeDecompositionAlgorithmFactory::instance().getTreeDecompositionAlgorithm();
        htd::ITreeDecomposition * decomp = algorithm->computeDecomposition(instance->internalGraph());
        delete algorithm;

        htd::IMutableTreeDecomposition * decompMutable = &(htd::TreeDecompositionFactory::instance().accessMutableTreeDecomposition(*decomp));

        HTDDecompositionPtr decomposition(decompMutable);
        return decomposition;
    }

} // namespace decomposer
