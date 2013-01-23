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

#include "libstrings.h"
#include "error.h"
#include "format.h"
#include <boost/filesystem.hpp>
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

const unsigned int LIBSTRINGS_VERSION_MAJOR = 1;
const unsigned int LIBSTRINGS_VERSION_MINOR = 1;
const unsigned int LIBSTRINGS_VERSION_PATCH = 1;

const char * extErrorString = NULL;

unsigned int c_error(const error& e) {
    extErrorString = ToNewCString(e.what());
    return e.code();
}

unsigned int c_error(const unsigned int code, const std::string& what) {
    return c_error(error(code, what.c_str()));
}


/*------------------------------
   Constants
------------------------------*/

/* The following are the possible codes that the library can return. */
const unsigned int LIBSTRINGS_OK                        = 0;
const unsigned int LIBSTRINGS_ERROR_INVALID_ARGS        = 1;
const unsigned int LIBSTRINGS_ERROR_NO_MEM              = 2;
const unsigned int LIBSTRINGS_ERROR_FILE_READ_FAIL      = 3;
const unsigned int LIBSTRINGS_ERROR_FILE_WRITE_FAIL     = 4;
const unsigned int LIBSTRINGS_ERROR_BAD_STRING          = 5;
const unsigned int LIBSTRINGS_RETURN_MAX                = LIBSTRINGS_ERROR_BAD_STRING;


/*------------------------------
   Version Functions
------------------------------*/

/* Returns whether this version of libstrings is compatible with the given
   version of libstrings. */
LIBSTRINGS bool IsCompatibleVersion(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch) {
    if (versionMajor == 1 && versionMinor == 1 && versionPatch <= 1)
        return true;
    else
        return false;
}

LIBSTRINGS void GetVersionNums(unsigned int * const versionMajor, unsigned int * const versionMinor, unsigned int * const versionPatch) {
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
LIBSTRINGS unsigned int GetLastErrorDetails(const char ** const details) {
    if (details == NULL)
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    *details = extErrorString;

    return LIBSTRINGS_OK;
}

LIBSTRINGS void CleanUpErrorDetails() {
    if (extErrorString != NULL) {
        delete [] extErrorString;
        extErrorString = NULL;
    }
}

/*----------------------------------
   Lifecycle Management Functions
----------------------------------*/

/* Opens a STRINGS, ILSTRINGS or DLSTRINGS file at path, returning a handle
   sh. If the strings file doesn't exist then a handle for a new file will be
   created. */
LIBSTRINGS unsigned int OpenStringsFile(strings_handle * const sh, const char * const path, const char * const fallbackEncoding) {
    if (sh == NULL || path == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    //Set the locale to get encoding conversions working correctly.
    setlocale(LC_CTYPE, "");
    locale global_loc = locale();
    locale loc(global_loc, new boost::filesystem::detail::utf8_codecvt_facet());
    boost::filesystem::path::imbue(loc);

    //Create handle.
    try {
        *sh = new _strings_handle_int(path, fallbackEncoding);
    } catch (error& e) {
        return c_error(e);
    }

    return LIBSTRINGS_OK;
}

/* Saves the strings associated with the given handle to the given path. */
LIBSTRINGS unsigned int SaveStringsFile(strings_handle sh, const char * const path) {
    if (sh == NULL || path == NULL)
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    try {
        sh->Save(path);
    } catch (error e) {
        return c_error(e);
    }

    return LIBSTRINGS_OK;
}

/* Closes the file associated with the given handle, freeing any memory
   allocated during its use. */
LIBSTRINGS void CloseStringsFile(strings_handle sh) {
    delete sh;
}


/*------------------------------
   String Reading Functions
------------------------------*/

/* Gets an array of all strings (with assigned IDs) in the file. */
LIBSTRINGS unsigned int GetStrings(strings_handle sh, string_data ** strings, size_t * numStrings) {
    if (sh == NULL || strings == NULL || numStrings == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    //Free memory if in use.
    if (sh->extStringDataArr != NULL) {
        for (size_t i=0; i < sh->extStringDataArrSize; i++)
            delete [] sh->extStringDataArr[i].data;
        delete [] sh->extStringDataArr;
        sh->extStringDataArr = NULL;
        sh->extStringDataArrSize = 0;
    }

    //Init values.
    *strings = NULL;
    *numStrings = 0;

    if (sh->data.empty())
        return LIBSTRINGS_OK;

    sh->extStringDataArrSize = sh->data.size();

    try {
        sh->extStringDataArr = new string_data[sh->extStringDataArrSize];
        size_t i=0;
        for (boost::unordered_map<uint32_t, string>::iterator it=sh->data.begin(), endIt=sh->data.end(); it != endIt; ++it) {
            sh->extStringDataArr[i].id = it->first;
            sh->extStringDataArr[i].data = ToNewCString(it->second);
            i++;
        }
    } catch (bad_alloc& e) {
        return c_error(LIBSTRINGS_ERROR_NO_MEM, e.what());
    } catch (error& e) {
        return c_error(e);
    }

    *strings = sh->extStringDataArr;
    *numStrings = sh->extStringDataArrSize;

    return LIBSTRINGS_OK;
}

/* Gets an array of any strings in the file that are not assigned IDs. */
LIBSTRINGS unsigned int GetUnreferencedStrings(strings_handle sh, char *** strings, size_t * numStrings) {
    if (sh == NULL || strings == NULL || numStrings == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    //Free memory if in use.
    if (sh->extStringArr != NULL) {
        for (size_t i=0; i < sh->extStringArrSize; i++)
            delete [] sh->extStringArr[i];
        delete [] sh->extStringArr;
        sh->extStringArr = NULL;
        sh->extStringArrSize = 0;
    }

    //Init values.
    *strings = NULL;
    *numStrings = 0;

    if (sh->unrefStrings.empty())
        return LIBSTRINGS_OK;

    //Allocate memory.
    sh->extStringArrSize = sh->unrefStrings.size();
    try {
        sh->extStringArr = new char*[sh->extStringArrSize];
        //Now loop through the offsets, getting the string for each.
        size_t i=0;
        for (boost::unordered_set<string>::iterator it=sh->unrefStrings.begin(), endIt=sh->unrefStrings.end(); it != endIt; ++it) {
            sh->extStringArr[i] = ToNewCString(*it);
            i++;
        }
    } catch (bad_alloc& e) {
        return c_error(LIBSTRINGS_ERROR_NO_MEM, e.what());
    } catch (error& e) {
        return c_error(e);
    }

    *strings = sh->extStringArr;
    *numStrings = sh->extStringArrSize;

    return LIBSTRINGS_OK;
}

/* Gets the string with the given ID from the file. */
LIBSTRINGS unsigned int GetString(strings_handle sh, const uint32_t stringId, char ** string) {
    if (sh == NULL || string == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    //Free memory in use.
    if (sh->extString != NULL) {
        delete [] sh->extString;
        sh->extString = NULL;
    }

    //Init value.
    *string = NULL;

    //Find string.
    try {
        boost::unordered_map<uint32_t, std::string>::iterator it = sh->data.find(stringId);
        if (it != sh->data.end())
            sh->extString = ToNewCString(it->second);
        else
            return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.");
    } catch (bad_alloc& e) {
        return c_error(LIBSTRINGS_ERROR_NO_MEM, e.what());
    } catch (error& e) {
        return c_error(e);
    }

    *string = sh->extString;

    return LIBSTRINGS_OK;
}


/*------------------------------
   String Writing Functions
------------------------------*/

/* Replaces all existing strings in the file with the given strings. */
LIBSTRINGS unsigned int SetStrings(strings_handle sh, const string_data * strings, const size_t numStrings) {
    if (sh == NULL || strings == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    boost::unordered_map<uint32_t, string> newMap;

    try {
        for (size_t i=0; i < numStrings; i++) {
            if (!newMap.insert(pair<uint32_t, string>(strings[i].id, strings[i].data)).second)
                return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "The ID given for the string \"" + string(strings[i].data) + "\" already exists.");
        }
    } catch (error& e) {
        return c_error(e);
    }

    sh->data = newMap;

    return LIBSTRINGS_OK;
}

/* Adds the given string to the file. */
LIBSTRINGS unsigned int AddString(strings_handle sh, const uint32_t stringId, const char * str) {
    if (sh == NULL || str == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    if (!sh->data.insert(pair<uint32_t, string>(stringId, str)).second)
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID already exists.");

    return LIBSTRINGS_OK;
}

/* Replaces the string corresponding to the given ID with the given string. */
LIBSTRINGS unsigned int EditString(strings_handle sh, const uint32_t stringId, const char * newString) {
    if (sh == NULL || newString == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    boost::unordered_map<uint32_t, string>::iterator it = sh->data.find(stringId);
    if (it == sh->data.end())
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.");

    it->second = newString;

    return LIBSTRINGS_OK;
}

/* Removes the string corresponding to the given ID. */
LIBSTRINGS unsigned int RemoveString(strings_handle sh, const uint32_t stringId) {
    if (sh == NULL) //Check for valid args.
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.");

    boost::unordered_map<uint32_t, string>::iterator it = sh->data.find(stringId);
    if (it == sh->data.end())
        return c_error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.");

    sh->data.erase(it);

    return LIBSTRINGS_OK;
}
