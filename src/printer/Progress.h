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

#include "../Printer.h"

namespace printer {

    class Progress : public Printer {
    public:
        Progress(Application& app, bool newDefault = false);

        virtual void inputInstance(const InstancePtr& instance) override;
        virtual void preprocessedInstance(const InstancePtr& instance) override;
        virtual void decomposerResult(const HTDDecompositionPtr& result) override;
        virtual void vertexOrdering(const std::vector<int>& ordering) override;
        virtual void beforeComputation() override;
        virtual void solverIntermediateEvent(const htd::vertex_t vertex, const Computation& computation, const std::string& message) override;
        virtual void solverIntermediateEvent(const htd::vertex_t vertex, const Computation& c1, const Computation& c2, const std::string& message) override;
        virtual void solverInvocationResult(const htd::vertex_t vertex, const Computation& computation) override;
        virtual void afterComputation() override;

    private:
        int tdComputedCount = 0;
    };

} // namespace printer
