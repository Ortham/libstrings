/*      libstrings

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

#ifndef __LIBSTRINGS_HELPERS_H__
#define __LIBSTRINGS_HELPERS_H__

#include <string>
#include <stdint.h>

namespace libstrings {
        // std::string to null-terminated uint8_t string converter.
        char * ToNewCString(const std::string& str);

        // Encoding conversions. 'encoding' can be of the form "Windows-*".
        // For ToUTF8, 'encoding' is actually the fallback encoding, and the
        // function first checks if the string is already valid UTF-8 before
        // doing anything.
        std::string ToUTF8(const std::string& str, const std::string& encoding);
        std::string FromUTF8(const std::string& str, const std::string& encoding);
}

#endif
