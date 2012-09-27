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

#ifndef LIBSTRINGS_H
#define LIBSTRINGS_H

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
//MSVC doesn't support C99, so do the stdbool.h definitions ourselves.
//START OF stdbool.h DEFINITIONS. 
#	ifndef __cplusplus
#		define bool	_Bool
#		define true	1
#		define false   0
#	endif
#	define __bool_true_false_are_defined   1
//END OF stdbool.h DEFINITIONS.
#else
#	include <stdbool.h>
#endif

// set up dll import/export decorators
// when compiling the dll on windows, ensure LIBLO_EXPORT is defined.  clients
// that use this header do not need to define anything to import the symbols
// properly.
#if defined(_WIN32) || defined(_WIN64)
#	ifdef LIBSTRINGS_STATIC
#		define LIBSTRINGS
#   elif defined LIBSTRINGS_EXPORT
#       define LIBSTRINGS __declspec(dllexport)
#   else
#       define LIBSTRINGS __declspec(dllimport)
#   endif
#else
#   define LIBSTRINGS
#endif

#ifdef __cplusplus
extern "C"
{
#endif


/*------------------------------
   Types
------------------------------*/

/* Abstraction of strings file info structure while providing type safety. */
typedef struct strings_handle_int * strings_handle;

/* Structure containing ID and data for a string. 
   Used by Get/SetStrings to ensure IDs and string data don't get mixed up. */
typedef struct {
	uint32_t id;
	uint8_t * data;
} string_data;

/* Return codes */
LIBSTRINGS extern const uint32_t LIBSTRINGS_OK;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_INVALID_ARGS;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_NO_MEM;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_FILE_NOT_FOUND;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_FILE_WRITE_FAIL;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_FILE_READ_FAIL;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_FILE_CORRUPT;
LIBSTRINGS extern const uint32_t LIBSTRINGS_ERROR_BAD_STRING;
LIBSTRINGS extern const uint32_t LIBSTRINGS_RETURN_MAX;
/* No doubt there will be more... */


/*------------------------------
   Version Functions
------------------------------*/

/* Returns whether this version of libstrings is compatible with the given
   version of libstrings. */
LIBSTRINGS bool IsCompatibleVersion (const uint32_t versionMajor, const uint32_t versionMinor, const uint32_t versionPatch);

/* Gets the version numbers for the libary. */
LIBSTRINGS void GetVersionNums(uint32_t * versionMajor, uint32_t * versionMinor, uint32_t * versionPatch);


/*------------------------------
   Error Handling Functions
------------------------------*/

/* Gets a string with details about the last error returned. */
LIBSTRINGS uint32_t GetLastErrorDetails(uint8_t ** details);

/* Frees the memory allocated to the last error details string. */
LIBSTRINGS void CleanUpErrorDetails();


/*----------------------------------
   Lifecycle Management Functions
----------------------------------*/

/* Opens a STRINGS, ILSTRINGS or DLSTRINGS file at path, returning a handle 
   sh. If the strings file doesn't exist then it will be created. The file 
   extension is used to determine the string data format used. */
LIBSTRINGS uint32_t OpenStringsFile(strings_handle * sh, const uint8_t * path);

/* Closes the file associated with the given handle. No changes are written
   until the handle is closed. Closing the handle also frees any memory 
   allocated during its use. If save is true and the file has been edited,
   the changes will be written to disk, otherwise they will be discarded. */
LIBSTRINGS void CloseStringsFile(strings_handle sh, const bool save);


/*------------------------------
   String Reading Functions
------------------------------*/

/* Gets an array of all strings (with assigned IDs) in the file. */
LIBSTRINGS uint32_t GetStrings(strings_handle sh, string_data ** strings, size_t * numStrings);

/* Gets an array of any strings in the file that are not assigned IDs. */
LIBSTRINGS uint32_t GetUnreferencedStrings(strings_handle sh, uint8_t *** strings, size_t * numStrings);

/* Gets the string with the given ID from the file. */
LIBSTRINGS uint32_t GetString(strings_handle sh, const uint32_t stringId, uint8_t ** string);


/*------------------------------
   String Writing Functions
------------------------------*/

/* Replaces all existing strings in the file with the given strings. */
LIBSTRINGS uint32_t SetStrings(strings_handle sh, const string_data * strings, const size_t numStrings);

/* Adds the given string to the file. */
LIBSTRINGS uint32_t AddString(strings_handle sh, const uint32_t stringId, const uint8_t * str);

/* Replaces the string corresponding to the given ID with the given string. */
LIBSTRINGS uint32_t EditString(strings_handle sh, const uint32_t stringId, const uint8_t * newString);

/* Removes the string corresponding to the given ID. */
LIBSTRINGS uint32_t RemoveString(strings_handle sh, const uint32_t stringId);

#ifdef __cplusplus
}
#endif

#endif