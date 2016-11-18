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

#pragma once

#include <vector>

#include "cuddObj.hh"
#include "TmpComputation.h"
#include "Application.h"
#include "TmpComputationManager.h"

class Application;

enum RESULT {
    SAT, UNSAT, UNDECIDED
};

class Solver {
public:

    Solver(const Application& app);

    // Return the computation for the current decomposition vertex
    virtual TmpComputation* compute(htd::vertex_t) = 0;

    virtual RESULT decide(const TmpComputation& c) = 0;

    virtual BDD solutions(const TmpComputation& c) = 0;

protected:
    const Application& app;
};
