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
const uint32_t LIBSTRINGS_OK                        = 0;
const uint32_t LIBSTRINGS_ERROR_INVALID_ARGS        = 1;
const uint32_t LIBSTRINGS_ERROR_NO_MEM              = 2;
const uint32_t LIBSTRINGS_ERROR_FILE_READ_FAIL      = 3;
const uint32_t LIBSTRINGS_ERROR_FILE_WRITE_FAIL     = 4;
const uint32_t LIBSTRINGS_ERROR_BAD_STRING          = 5;
const uint32_t LIBSTRINGS_RETURN_MAX                = LIBSTRINGS_ERROR_BAD_STRING;


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
    if (extErrorString != NULL) {
        delete [] extErrorString;
        extErrorString = NULL;
    }

    try {
        extErrorString = ToUint8_tString(lastException.what());
    } catch (bad_alloc /*&e*/) {
        return error(LIBSTRINGS_ERROR_NO_MEM).code();
    }

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

/* Saves the strings associated with the given handle to the given path. */
LIBSTRINGS uint32_t SaveStringsFile(strings_handle sh, const uint8_t * path) {
    if (sh == NULL || path == NULL)
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

    try {
        sh->Save(string(reinterpret_cast<const char *>(path)));
    } catch (error e) {
        return e.code();
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
            sh->extStringDataArr[i].data = sh->GetString(it->second);
            i++;
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
    if (sh == NULL || strings == NULL || numStrings == NULL) //Check for valid args.
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

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
        sh->extStringArr = new uint8_t*[sh->extStringArrSize];
        //Now loop through the offsets, getting the string for each.
        size_t i=0;
        for (boost::unordered_set<string>::iterator it=sh->unrefStrings.begin(), endIt=sh->unrefStrings.end(); it != endIt; ++it) {
            sh->extStringArr[i] = sh->GetString(*it);
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
            sh->extString = sh->GetString(it->second);
        else
            return error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.").code();
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
    if (sh == NULL || strings == NULL) //Check for valid args.
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

    boost::unordered_map<uint32_t, string> newMap;

    try {
        for (size_t i=0; i < numStrings; i++) {
            string str = sh->trans.Utf8ToEnc(string(reinterpret_cast<const char *>(strings[i].data)));
            if (!newMap.insert(pair<uint32_t, string>(strings[i].id, str)).second)
                return error(LIBSTRINGS_ERROR_INVALID_ARGS, string("The ID given for the string \"") + str + string("\" already exists.")).code();
        }
    } catch (error& e) {
        return e.code();
    }

    sh->data = newMap;

    return LIBSTRINGS_OK;
}

/* Adds the given string to the file. */
LIBSTRINGS uint32_t AddString(strings_handle sh, const uint32_t stringId, const uint8_t * str) {
    if (sh == NULL || str == NULL) //Check for valid args.
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

    //Convert to Windows-1252.
    string strString;
    try {
        strString = sh->trans.Utf8ToEnc(string(reinterpret_cast<const char *>(str)));
    } catch (error& e) {
        return e.code();
    }

    if (!sh->data.insert(pair<uint32_t, string>(stringId, strString)).second)
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID already exists.").code();

    return LIBSTRINGS_OK;
}

/* Replaces the string corresponding to the given ID with the given string. */
LIBSTRINGS uint32_t EditString(strings_handle sh, const uint32_t stringId, const uint8_t * newString) {
    if (sh == NULL || newString == NULL) //Check for valid args.
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

    //Convert to Windows-1252.
    string strString;
    try {
        strString = sh->trans.Utf8ToEnc(string(reinterpret_cast<const char *>(newString)));
    } catch (error& e) {
        return e.code();
    }

    boost::unordered_map<uint32_t, string>::iterator it = sh->data.find(stringId);
    if (it == sh->data.end())
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.").code();

    it->second = strString;

    return LIBSTRINGS_OK;
}

/* Removes the string corresponding to the given ID. */
LIBSTRINGS uint32_t RemoveString(strings_handle sh, const uint32_t stringId) {
    if (sh == NULL) //Check for valid args.
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "Null pointer passed.").code();

    boost::unordered_map<uint32_t, string>::iterator it = sh->data.find(stringId);
    if (it == sh->data.end())
        return error(LIBSTRINGS_ERROR_INVALID_ARGS, "The given ID does not exist.").code();

    sh->data.erase(it);

    return LIBSTRINGS_OK;
}
