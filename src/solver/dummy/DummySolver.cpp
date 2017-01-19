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


#include "DummySolver.h"
#include "cuddObj.hh"

namespace solver {
    namespace dummy {

        DummySolver::DummySolver(const Application& app)
        : ::Solver(app) {
        }

        Computation* DummySolver::compute(htd::vertex_t root) {
            std::vector<BDD> cubesAtLevels;
            Computation* c = app.getNSFManager().newComputation(app.getInputInstance()->getQuantifierSequence(), cubesAtLevels, app.getBDDManager().getManager().bddOne());
            return c;
        }
    }
} // namespace solver::dummy
