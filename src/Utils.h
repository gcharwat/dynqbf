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

#include <sstream>
#include <stdexcept>
#include <vector>


namespace std {

    ostream & operator<<(std::ostream & stream, const std::vector<bool> & input);

}

namespace utils {

    inline int strToInt(const std::string& str, const std::string& errorMsg) {
        int number;
        if (!(std::istringstream(str) >> number)) {
            throw std::runtime_error(errorMsg);
        }
        return number;
    }

} // namespace utils


// for compatibility with cygwin
namespace std {

#ifndef to_string // gcc patch to_string

    template <typename T>
    string to_string(T value) {
        std::ostringstream os;
        os << value;
        return os.str();
    }
#endif
}


// substracts b<T> to a<T>

template <typename T>
void
substract_vector(std::vector<T>& a, const std::vector<T>& b) {
    typename std::vector<T>::iterator it = a.begin();
    typename std::vector<T>::const_iterator it2 = b.begin();
    typename std::vector<T>::iterator end = a.end();
    typename std::vector<T>::const_iterator end2 = b.end();

    while (it != end) {
        while (it2 != end2) {
            if (*it == *it2) {
                it = a.erase(it);
                end = a.end();
                it2 = b.begin();
            } else
                ++it2;
        }
        ++it;
        it2 = b.begin();
    }
}