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

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

#include "Solver.h"

using namespace std;

class AbortException : public runtime_error {
public:

    AbortException(const char* what, RESULT result)
    : runtime_error(what), result(result) {
    }

    RESULT getResult() const {
        return result;
    }


private:
    RESULT result;

    static ostringstream cnvt;
};
