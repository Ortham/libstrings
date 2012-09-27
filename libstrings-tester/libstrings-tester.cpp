/*	libstrings
	
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

#include "libstrings.h"

#include <iostream>
#include <stdint.h>
#include <fstream>
#include <clocale>
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/filesystem.hpp>

using namespace std;
namespace fs = boost::filesystem;

int main() {
	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	std::locale global_loc = std::locale();
	std::locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	strings_handle sh;
	uint8_t * path = reinterpret_cast<uint8_t *>("C:/Program Files (x86)/Steam/steamapps/common/skyrim/Data/Strings/Skyrim_English.STRINGS");
	uint32_t ret;
	string_data * dataArr;
	size_t arrSize;
	uint8_t * str;
	uint32_t id;
	uint8_t ** stringArr;

	std::ofstream out("libstrings-tester.txt");
	if (!out.good()){
		cout << "File could not be opened for reading.";
		exit(1);
	}

	out << "Using path: " << path << endl;

	out << "TESTING OpenStringsFile(...)" << endl;
	ret = OpenStringsFile(&sh, path);
	if (ret != LIBSTRINGS_OK)
		out << '\t' << "OpenStringsFile(...) failed! Return code: " << ret << endl;
	else
		out << '\t' << "OpenStringsFile(...) successful!" << endl;

	out << "TESTING GetStrings(...)" << endl;
	ret = GetStrings(sh, &dataArr, &arrSize);
	if (ret != LIBSTRINGS_OK)
		out << '\t' << "GetStrings(...) failed! Return code: " << ret << endl;
	else {
		out << '\t' << "GetStrings(...) successful! Number of strings: " << arrSize << endl;
		out << '\t' << "ID" << '\t' << "String" << endl;
		for (size_t i=0; i < arrSize; i++) {
			out << dataArr[i].id << '\t' << dataArr[i].data << endl;
		}
	}

	id = dataArr[500].id;
	
	out << "TESTING GetString(...)" << endl;
	ret = GetString(sh, id, &str);
	if (ret != LIBSTRINGS_OK)
		out << '\t' << "GetString(...) failed! Return code: " << ret << endl;
	else {
		out << '\t' << "GetString(...) successful!"  << endl;
		out << '\t' << "String fetched: " << str << endl;
	}
	
	out << "TESTING GetUnreferencedStrings(...)" << endl;
	ret = GetUnreferencedStrings(sh, &stringArr, &arrSize);
	if (ret != LIBSTRINGS_OK)
		out << '\t' << "GetUnreferencedStrings(...) failed! Return code: " << ret << endl;
	else {
		out << '\t' << "GetUnreferencedStrings(...) successful! Number of strings: " << arrSize << endl;
		for (size_t i=0; i < arrSize; i++) {
			out << '\t' << stringArr[i] << endl;
		}
	}

	out << "TESTING CloseStringsFile(...)" << endl;
	CloseStringsFile(sh, true);

	out.close();
	return 0;
}