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

#include <string>
#include <vector>

namespace options {

    class Condition;

    class Option {
    public:
        Option(const std::string& name, const std::string& description);
        virtual ~Option();

        const std::string& getName() const {
            return name;
        }

        const std::string& getDashedName() const {
            return dashedName;
        }

        const std::string& getDescription() const {
            return description;
        }

        // Whether this option has been used (at least once)

        bool isUsed() const {
            return used;
        }

        void setUsed() {
            used = true;
        }

        // Adds a condition that must be satisfied for this option to be used.
        void addCondition(const Condition& condition);

        // If this option has been used but any added condition is unsatisfied, throws an exception.
        virtual void checkConditions() const;

        virtual void printHelp() const;

    protected:
        static const int NAME_WIDTH = 23;

        typedef std::vector<const Condition*> Conditions;
        Conditions conditions;

    private:
        const std::string name;
        const std::string dashedName;
        const std::string description; // Description of the option in the help message
        bool used;
    };

} // namespace options
