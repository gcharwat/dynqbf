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

#include "CombinedPreprocessor.h"
#include "CNF3Preprocessor.h"
#include "../Application.h"
#include "UnitLiteralPreprocessor.h"

#include <htd/main.hpp>


namespace preprocessor {

    const std::string CombinedPreprocessor::OPTION_SECTION = "Preprocessing options";

    CombinedPreprocessor::CombinedPreprocessor(Application& app, bool newDefault)
    : Preprocessor(app, "combined", "Combine various preprocessing strategies", newDefault),
    optUseCNF3("cnf3", "convert to 3-CNF"),
    optUseUnitLiteral("unit-literal", "apply unit literal elimination"){
        app.getOptionHandler().addOption(optUseCNF3, OPTION_SECTION);
        app.getOptionHandler().addOption(optUseUnitLiteral, OPTION_SECTION);
    }

    InstancePtr CombinedPreprocessor::preprocess(const InstancePtr& instance) const {
        InstancePtr preprocessed = instance;
        
        if (optUseUnitLiteral.isUsed()) {
            UnitLiteralPreprocessor* unitLiteralPreprocessor = new UnitLiteralPreprocessor(app);
            preprocessed = unitLiteralPreprocessor->preprocess(preprocessed);
            delete unitLiteralPreprocessor;
        }
        
        if (optUseCNF3.isUsed()) {
            CNF3Preprocessor* cnf3Preprocessor = new CNF3Preprocessor(app, false);
            preprocessed = cnf3Preprocessor->preprocess(preprocessed);
            delete cnf3Preprocessor;
        }

        return preprocessed;
    }

} // namespace preprocessor
