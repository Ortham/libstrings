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

#include "format.h"
#include "libstrings.h"
#include "exception.h"
#include "helpers.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace libstrings;

namespace fs = boost::filesystem;

strings_handle_int::strings_handle_int(string filePath) :
	path(filePath),
	extStringDataArr(NULL),
	extStringArr(NULL),
	extString(NULL),
	extStringDataArrSize(0),
	extStringArrSize(0), 
	Header() {

	//Check extension.
	const string ext = fs::path(filePath).extension().string();
	if (ext == ".STRINGS")
		isDotStrings = true;
	else if (ext == ".DLSTRINGS" || ext == ".ILSTRINGS")
		isDotStrings = false;
	else
		throw error(LIBSTRINGS_ERROR_INVALID_ARGS, "File passed does not have a valid extension.");

	if (fs::exists(path)) {
		ifstream in(path.c_str(), ios::binary);
		if (!in.good())
			throw error(LIBSTRINGS_ERROR_FILE_READ_FAIL, filePath);

		//Read directory count into Header::count.
		in.read((char*)&count, sizeof(uint32_t) / sizeof(char));

		//Read string data size into Header::dataSize.
		in.read((char*)&dataSize, sizeof(uint32_t) / sizeof(char));

		//Now allocate memory to Header::directory and Header::data with the obtained sizes.
		try {
			directory = new DirectoryEntry[count];
			data = new uint8_t[dataSize];
		} catch (bad_alloc /*&e*/) {
			throw error(LIBSTRINGS_ERROR_NO_MEM);
		}

		//Read directory into Header::directory.
		in.read((char*)directory, sizeof(DirectoryEntry) * count / sizeof(char));

		//Read string data into Header::data.
		in.read((char*)data, sizeof(uint8_t) * dataSize / sizeof(char));
	}

	trans.SetEncoding(1252);
}

strings_handle_int::~strings_handle_int() {
	//Save everything in memory to the file. This could be done easier in C...
	ofstream out((path + ".new").c_str(), ios::binary | ios::trunc);

	out << string((char*)&count, sizeof(uint32_t)) 
		<< string((char*)&dataSize, sizeof(uint32_t))
		<< string((char*)directory, sizeof(DirectoryEntry) * count)
		<< string((char*)data, sizeof(uint8_t) * dataSize);

	out.close();

	delete [] directory;
	delete [] data;

	delete [] extString;

	for (size_t i=0; i < extStringDataArrSize; i++)
		delete [] extStringDataArr[i].data;
	delete [] extStringDataArr;

	for (size_t i=0; i < extStringArrSize; i++)
		delete [] extStringArr[i];
	delete [] extStringArr;
}

uint8_t * strings_handle_int::GetStringFromData(size_t offset) {
	//Strings are null terminated, so scan through until that is reached. Alternatively, use stringstream's << operator to do that for us.
	ostringstream out;
	if (isDotStrings)
		out << data + offset;
	else
		out << data + offset + sizeof(uint32_t);

	return ToUint8_tString(trans.EncToUtf8(out.str()));
}

void strings_handle_int::FindUnreferencedStrings() {
	/*This is a bit tricky.
	The directory has a list of offsets for strings with IDs.
	There may be unreferenced strings scattered between these strings.
	Strings are all null-terminated.
	Directory entries are not ordered, so offsets are not ordered, so if
	we just moved along the offsets we might miss unreferenced strings.

	Instead, read through the directory, putting all offsets into a hashset.
	Then read through the data block. Each time a new string is encountered,
	search the hashset for its offset. If not found, add it to a vector for
	outputting later.
	*/

	boost::unordered_set<uint32_t> offsets;
	for (size_t i=0; i < count; i++) {
		offsets.emplace(directory[i].offset);
	}

	uint32_t offset = 0;
	while (offset < dataSize) {
		if (offsets.find(offset) == offsets.end())
			unrecOffsets.emplace(offset);

		//If !isDotStrings, the length data can be used to calculate the next offset.
		if (!isDotStrings)
			offset += *(uint32_t*)(data + offset) + sizeof(uint32_t);  //First 4 bytes of data at the offset is the string length.
		else {
			//Otherwise, we can either count until one after the next null, or we can use stringstream.
			ostringstream stringHolder;
			stringHolder << data + offset;
			offset += stringHolder.str().length() + 1;  //+1 is for the null character.
		}
	}
}