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

#include "../Decomposer.h"
#include "../options/Choice.h"
#include "../options/DefaultIntegerValueOption.h"
#include "../Instance.h"

#include <list>

namespace decomposer {

    class HTDTreeDecomposer : public Decomposer {
    public:
        HTDTreeDecomposer(Application& app, bool newDefault = false);

        HTDDecompositionPtr decompose(const InstancePtr& instance) const override;
        
    protected:
        void printStatistics(const htd::IMultiHypergraph & graph, const htd::ITreeDecomposition & decomposition) const;

    private:
        static const std::string OPTION_SECTION;

        options::Choice optGraphPreprocessing;
        options::Choice optNormalization;
        options::Choice optEliminationOrdering;
//        options::Option optJoinCompression;
        options::Option optNoEmptyRoot;
        options::Option optEmptyLeaves;
        options::Option optPathDecomposition;
        options::Choice optRootSelectionFitnessFunction;
        options::DefaultIntegerValueOption optRootSelectionIterations;
        options::Choice optDecompositionFitnessFunction;
        options::DefaultIntegerValueOption optDecompositionIterations;
        
        options::Option optPrintStats;
    };

} // namespace decomposer
