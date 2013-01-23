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

#ifndef __LIBSTRINGS_ERROR_H__
#define __LIBSTRINGS_ERROR_H__

#include <exception>
#include <string>

namespace libstrings {

    class error : public std::exception {
    public:
        error(const unsigned int code, const std::string& what) : _code(code), _what(what) {}
        ~error() throw() {};

        unsigned int code() const { return _code; }
        const char * what() const throw() { return _what.c_str(); }
    private:
        std::string _what;
        unsigned int _code;
    };
}

#endif
