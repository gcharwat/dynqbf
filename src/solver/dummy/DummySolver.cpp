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


#include "DummySolver.h"
#include "cuddObj.hh"

namespace solver {
    namespace dummy {

        DummySolver::DummySolver(const Application& app)
        : ::Solver(app) {
        }

        Computation* DummySolver::compute(htd::vertex_t root) {
            Computation* c = app.getNSFManager().newComputation(app.getBDDManager().getManager().bddOne());
            return c;
        }

        RESULT DummySolver::decide(const Computation& c) {
            return RESULT::UNDECIDED;
        }

        BDD DummySolver::solutions(const Computation& c) {
            throw std::runtime_error("not implemented");
        }

    }
} // namespace solver::dummy
