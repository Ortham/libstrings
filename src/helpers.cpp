/*  libstrings

    A library for reading and writing STRINGS, ILSTRINGS and DLSTRINGS files.

    Copyright (C) 2012    WrinklyNinja

    This file is part of libstrings.

    libstrings is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version.

    libstrings is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libstrings.  If not, see
    <http://www.gnu.org/licenses/>.
*/

#include "helpers.h"
#include "libstrings.h"
#include "error.h"
#include <cstring>
#include <boost/locale.hpp>

using namespace std;

namespace libstrings {

    // std::string to null-terminated char string converter.
    char * ToNewCString(const std::string& str) {
        char * p = new char[str.length() + 1];
        return strcpy(p, str.c_str());
    }

    std::string ToUTF8(const std::string& str, const std::string& encoding) {
        try {
            return boost::locale::conv::to_utf<char>(str, encoding, boost::locale::conv::stop);
        } catch (boost::locale::conv::conversion_error& e) {
            throw error(LIBSTRINGS_ERROR_BAD_STRING, "\"" + str + "\" cannot be encoded in " + encoding + ".");
        }
    }

    std::string FromUTF8(const std::string& str, const std::string& encoding) {
        try {
            return boost::locale::conv::from_utf<char>(str, encoding, boost::locale::conv::stop);
        } catch (boost::locale::conv::conversion_error& e) {
            throw error(LIBSTRINGS_ERROR_BAD_STRING, "\"" + str + "\" cannot be encoded in " + encoding + ".");
        }
    }
}
