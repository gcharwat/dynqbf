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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "DefaultIntegerValueOption.h"

namespace options {

    DefaultIntegerValueOption::DefaultIntegerValueOption(const std::string& name, const std::string& placeholder, const std::string& description, int defaultValue)
    : ValueOption(name, placeholder, description)
    , value(defaultValue) 
    , defaultValue(defaultValue)
    {
    }

    void DefaultIntegerValueOption::setValue(const std::string& v) {
        int converted;
        if (!(std::istringstream(v) >> converted)) {
            std::ostringstream ss;
            ss << "Option '" << getName() << "' expects an integer as argument.";
            throw std::runtime_error(ss.str());
        }
        
        if (isUsed() && value != converted) {
            std::ostringstream ss;
            ss << "Option '" << getName() << "' only takes a single value, but more than one was specified.";
            throw std::runtime_error(ss.str());
        }
        value = converted;
    }
    
    void DefaultIntegerValueOption::printHelp() const {
        std::ostringstream field;
        field << getDashedName() << " <" << placeholder << '>';
        std::cerr << "  " << std::left << std::setw(NAME_WIDTH) << field.str() << " : " << getDescription() << " (Default: " << getDefaultValue() << ")" << std::endl;
    }


} // namespace options
