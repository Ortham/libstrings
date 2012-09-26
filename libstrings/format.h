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

#ifndef LIBSTRINGS_FORMAT_H
#define LIBSTRINGS_FORMAT_H

#include "libstrings.h"
#include "helpers.h"
#include <stdint.h>
#include <string>
#include <boost/unordered_set.hpp>

namespace libstrings {

	//See here for format details: http://www.uesp.net/wiki/Tes5Mod:String_Table_File_Format
	
	struct StringData {
		uint32_t length;				//Length of data. Not present in STRINGS files, but is present in ILSTRINGS and DLSTRINGS.
		uint8_t * data;					//Null-terminated C string.
	};

	struct DirectoryEntry {
		DirectoryEntry() : id(0), offset(0) {}
		uint32_t id;					//The ID of the entry's string, used by plugins to reference the string.
		uint32_t offset;				//The offset at which the string is found, relative to the start of raw string data.
	};

	struct Header {
		Header() : 
		count(0),
		dataSize(0),
		directory(NULL),
		data(NULL) {}

		uint32_t count;					//Number of entries in the string table.
		uint32_t dataSize;				//Size of the string data following the header and directory.
		DirectoryEntry * directory;		//An array of DirectoryEntry objects, of number given by 'count'. The objects are not required to be sequential.
		uint8_t * data;					//The raw string data, of length dataSize;
	};
}

//File data is stored in its original encoding. 
//Data strings are converted to and from UTF-8 when being obtained from input and sent to output.
struct strings_handle_int : public libstrings::Header {
	strings_handle_int(std::string filePath);
	~strings_handle_int();

	std::string path;
	bool isDotStrings;		//Used to determine whether or not to look for lengths in the raw string data. Set according to file extension.

	string_data * extStringDataArr;
	uint8_t ** extStringArr;
	uint8_t * extString;
	
	size_t extStringDataArrSize;
	size_t extStringArrSize;

	libstrings::Transcoder trans;

	boost::unordered_set<uint32_t> unrecOffsets;

	uint8_t * GetStringFromData(size_t offset);
	void FindUnreferencedStrings();
};

#endif