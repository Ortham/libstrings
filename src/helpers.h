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

#ifndef LIBSTRINGS_HELPERS_H
#define LIBSTRINGS_HELPERS_H

#include <string>
#include <boost/unordered_map.hpp>
#include <boost/filesystem.hpp>

namespace libstrings {
        // std::string to null-terminated uint8_t string converter.
        uint8_t * ToUint8_tString(std::string str);

        // converts between encodings.
        // need to be able to convert Windows-1251 -> UTF-8 and Windows-1252 ->
        // UTF-8.
        class Transcoder {
        private:
            boost::unordered_map<char, uint32_t> map1251toUTF8; //Windows-1251, UTF-8.
            boost::unordered_map<char, uint32_t> map1252toUTF8; //Windows-1252, UTF-8.
        public:
            Transcoder();
            std::string ToUTF8(const std::string inString, int inEncoding);
        };
}

#endif
