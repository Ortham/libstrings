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
#include "exception.h"
#include "format.h"
#include <boost/filesystem/detail/utf8_codecvt_facet.hpp>
#include <boost/unordered_set.hpp>
#include <locale>
#include <sstream>
#include <vector>

using namespace std;
using namespace libstrings;

/*------------------------------
   Global variables
------------------------------*/

const uint32_t LIBSTRINGS_VERSION_MAJOR = 1;
const uint32_t LIBSTRINGS_VERSION_MINOR = 0;
const uint32_t LIBSTRINGS_VERSION_PATCH = 0;

uint8_t * extErrorString = NULL;


/*------------------------------
   Constants
------------------------------*/

/* The following are the possible codes that the library can return. */
LIBSTRINGS const uint32_t LIBSTRINGS_OK						= 0;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_INVALID_ARGS		= 1;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_NO_MEM			= 2;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_FILE_READ_FAIL	= 3;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_FILE_WRITE_FAIL	= 4;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_FILE_NOT_FOUND	= 5;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_FILE_CORRUPT		= 6;
LIBSTRINGS const uint32_t LIBSTRINGS_ERROR_BAD_STRING		= 7;
LIBSTRINGS const uint32_t LIBSTRINGS_RETURN_MAX				= LIBSTRINGS_ERROR_BAD_STRING;


/*------------------------------
   Version Functions
------------------------------*/

/* Returns whether this version of libstrings is compatible with the given
   version of libstrings. */
LIBSTRINGS bool IsCompatibleVersion(const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch) {
	if (versionMajor == 1 && versionMinor == 0 && versionPatch == 0)
		return true;
	else
		return false;
}

LIBSTRINGS void GetVersionNums(uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch) {
	*versionMajor = LIBSTRINGS_VERSION_MAJOR;
	*versionMinor = LIBSTRINGS_VERSION_MINOR;
	*versionPatch = LIBSTRINGS_VERSION_PATCH;
}


/*------------------------------
   Error Handling Functions
------------------------------*/

/* Outputs a string giving the a message containing the details of the
   last error or warning encountered by a function called for the given
   game handle. */
LIBSTRINGS uint32_t GetLastErrorDetails(uint8_t ** details) {
	if (details == NULL)
		return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	//Free memory if in use.
	delete [] extErrorString;
	extErrorString = NULL;

	try {
		extErrorString = ToUint8_tString(lastException.what());
	} catch (bad_alloc /*&e*/) {
		return error(LIBSTRINGS_ERROR_NO_MEM).code();
	}

	*details = extErrorString;
	return LIBSTRINGS_OK;
}

LIBSTRINGS void CleanUpErrorDetails() {
	delete [] extErrorString;
	extErrorString = NULL;
}

/*----------------------------------
   Lifecycle Management Functions
----------------------------------*/

/* Opens a STRINGS, ILSTRINGS or DLSTRINGS file at path, returning a handle 
   sh. If the strings file doesn't exist then it will be created. The file 
   extension is used to determine the string data format used. */
LIBSTRINGS uint32_t OpenStringsFile(strings_handle * sh, const uint8_t * path) {
	if (sh == NULL || path == NULL) //Check for valid args.
		return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	//Set the locale to get encoding conversions working correctly.
	setlocale(LC_CTYPE, "");
	locale global_loc = locale();
	locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
	boost::filesystem::path::imbue(loc);

	//Create handle.
	try {
		*sh = new strings_handle_int(string(reinterpret_cast<const char *>(path)));
	} catch (error& e) {
		return e.code();
	}

	return LIBSTRINGS_OK;
}

/* Closes the file associated with the given handle. No changes are written
   until the handle is closed. Closing the handle also frees any memory 
   allocated during its use. */
LIBSTRINGS void CloseStringsFile(strings_handle sh) {
	delete sh;
}


/*------------------------------
   String Reading Functions
------------------------------*/

/* Gets an array of all strings (with assigned IDs) in the file. */
LIBSTRINGS uint32_t GetStrings(strings_handle sh, string_data ** strings, size_t * numStrings) {
	if (sh == NULL || strings == NULL || numStrings == NULL) //Check for valid args.
		return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();
	
	//Free memory if in use.
	if (sh->extStringDataArr != NULL) {
		for (size_t i=0; i < sh->extStringDataArrSize; i++)
			delete [] sh->extStringDataArr[i].data;
		delete [] sh->extStringDataArr;
		sh->extStringDataArr = NULL;
		sh->extStringDataArrSize = 0;
	}

	sh->extStringDataArrSize = sh->count;

	try {
		sh->extStringDataArr = new string_data[sh->extStringDataArrSize];
		for (size_t i=0; i < sh->count; i++) {
			sh->extStringDataArr[i].id = sh->directory[i].id;
			sh->extStringDataArr[i].data = sh->GetStringFromData(sh->directory[i].offset);
		}
	} catch (bad_alloc& /*e*/) {
		return error(LIBSTRINGS_ERROR_NO_MEM).code();
	} catch (error& e) {
		return e.code();
	}

	*strings = sh->extStringDataArr;
	*numStrings = sh->extStringDataArrSize;

	return LIBSTRINGS_OK;
}

/* Gets an array of any strings in the file that are not assigned IDs. */
LIBSTRINGS uint32_t GetUnreferencedStrings(strings_handle sh, uint8_t *** strings, size_t * numStrings) {

	//Free memory if in use.
	if (sh->extStringArr != NULL) {
		for (size_t i=0; i < sh->extStringArrSize; i++)
			delete [] sh->extStringArr[i];
		delete [] sh->extStringArr;
		sh->extStringArr = NULL;
		sh->extStringArrSize = 0;
	}

	//Find the offsets for all unreferenced strings.
	if (sh->unrecOffsets.empty())
		sh->FindUnreferencedStrings();

	//Allocate memory.
	sh->extStringArrSize = sh->unrecOffsets.size();
	try {
		sh->extStringArr = new uint8_t*[sh->extStringArrSize];
		//Now loop through the offsets, getting the string for each.
		size_t i=0;
		for (boost::unordered_set<uint32_t>::iterator it=sh->unrecOffsets.begin(), endIt=sh->unrecOffsets.end(); it != endIt; ++it) {
			sh->extStringArr[i] = sh->GetStringFromData(*it);
			i++;
		}
	} catch (bad_alloc& /*e*/) {
		return error(LIBSTRINGS_ERROR_NO_MEM).code();
	} catch (error& e) {
		return e.code();
	}
	
	*strings = sh->extStringArr;
	*numStrings = sh->extStringArrSize;
	
	return LIBSTRINGS_OK;
}

/* Gets the string with the given ID from the file. */
LIBSTRINGS uint32_t GetString(strings_handle sh, const uint32_t stringId, uint8_t ** string) {
	if (sh == NULL || string == NULL) //Check for valid args.
		return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

	//Free memory in use.
	delete [] sh->extString;

	//Find offset corresponding to given ID.
	uint32_t offset;
	for (size_t i=0; i < sh->count; i++) {
		if (sh->directory[i].id == stringId) {
			offset = sh->directory[i].offset;
			break;
		}
	}

	try {
		sh->extString = sh->GetStringFromData(offset);
	} catch (bad_alloc /*&e*/) {
		return error(LIBSTRINGS_ERROR_NO_MEM).code();
	} catch (error& e) {
		return e.code();
	}

	*string = sh->extString;

	return LIBSTRINGS_OK;
}


/*------------------------------
   String Writing Functions
------------------------------*/

/* Replaces all existing strings in the file with the given strings. */
LIBSTRINGS uint32_t SetStrings(strings_handle sh, const string_data * strings, const size_t numStrings) {

}

/* Adds the given string to the file. */
LIBSTRINGS uint32_t AddString(strings_handle sh, const uint32_t stringId, const uint8_t * string) {

}

/* Replaces the string corresponding to the given ID with the given string. */
LIBSTRINGS uint32_t EditString(strings_handle sh, const uint32_t stringId, const uint8_t * newString) {

}

/* Removes the string corresponding to the given ID. */
LIBSTRINGS uint32_t RemoveString(strings_handle sh, const uint32_t stringId) {

}

/* Removes any strings in the file that exist as raw data but do not have IDs assigned to them. */
LIBSTRINGS uint32_t RemoveUnreferencedStrings(strings_handle sh) {

}