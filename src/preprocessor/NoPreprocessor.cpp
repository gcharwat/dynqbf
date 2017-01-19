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

#include "NoPreprocessor.h"
#include "../Application.h"

#include <htd/main.hpp>


namespace preprocessor {

    NoPreprocessor::NoPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "none", "No preprocessing", newDefault) {
    }

    InstancePtr NoPreprocessor::preprocess(const InstancePtr& instance) const {
        return instance;
    }

} // namespace preprocessor
