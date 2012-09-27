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
#include <map>

/* See here for format details: http://www.uesp.net/wiki/Tes5Mod:String_Table_File_Format
   File data is stored in its original encoding. 
   Data strings are converted to and from UTF-8 when being obtained from input and sent to output. */
struct strings_handle_int {
	strings_handle_int(std::string filePath);
	~strings_handle_int();

	//File metadata.
	std::string path;
	bool isDotStrings;		//Used to determine whether or not to look for lengths in the raw string data. Set according to file extension.
	bool isEdited;			//Init to false, true if an editing function is called, decides whether to disk write on destruct.

	//File data.
	std::map<uint32_t, std::string> data;	//Internal data storage. uint32_t is the string id and std::string is the string itself.

	//External data pointers.
	string_data * extStringDataArr;
	uint8_t ** extStringArr;
	uint8_t * extString;
	
	//External data array sizes.
	size_t extStringDataArrSize;
	size_t extStringArrSize;

	//Transcoder for UTF-8 <-> Windows-1252 conversions.
	libstrings::Transcoder trans;

	//All the unreferenced strings in the file.
	boost::unordered_set<std::string> unrefStrings;

	//Used to generate library output strings.
	uint8_t * GetString(std::string str);
};

#endif