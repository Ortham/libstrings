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

/**
    @file libstrings.h
    @brief This file contains the API frontend.

    @note libstrings is *not* thread safe. Thread safety is a goal, but one that has not yet been achieved. Bear this in mind if using it in a multi-threaded client.

    @section var_sec Variable Types

    libstrings uses character strings and integers for information input/output.
      - All strings are null-terminated byte character strings encoded in UTF-8.
      - All return, game and load order method codes are unsigned integers at least 16 bits in size.
      - All array sizes are unsigned integers at least 16 bits in size.
      - File paths are case-sensitive if and only if the underlying file system is case-sensitive.

    @section memory_sec Memory Management

    libloadorder manages the memory of strings and arrays it returns internally, so such strings and arrays should not be deallocated by the client.

    Data returned by a function lasts until a function is called which returns data of the same type (eg. a string is stored until the client calls another function which returns a string, an integer array lasts until another integer array is returned, etc.).

    All allocated memory is freed when st_close() is called, except the string allocated by st_get_error_message(), which must be freed by calling st_cleanup().
*/

#ifndef __LIBSTRINGS_H__
#define __LIBSTRINGS_H__

#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
//MSVC doesn't support C99, so do the stdbool.h definitions ourselves.
//START OF stdbool.h DEFINITIONS.
#       ifndef __cplusplus
#               define bool     _Bool
#               define true     1
#               define false   0
#       endif
#       define __bool_true_false_are_defined   1
//END OF stdbool.h DEFINITIONS.
#else
#       include <stdbool.h>
#endif

// set up dll import/export decorators
// when compiling the dll on windows, ensure LIBSTRINGS_EXPORT is defined.  clients
// that use this header do not need to define anything to import the symbols
// properly.
#if defined(_WIN32) || defined(_WIN64)
#       ifdef LIBSTRINGS_STATIC
#               define LIBSTRINGS
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

/**
    @brief A structure that holds all game-specific data used by libloadorder.
    @details Used to keep each strings file's data independent. Abstracts the definition of libstrings' internal state while still providing type safety across the library's functions. Multiple handles can also be made for each strings file, though it should be kept in mind that libstrings is not thread-safe.
*/
typedef struct _strings_handle_int * st_strings_handle;

/**
    @brief A structure holding the ID and corresponding data of a string.
    @details Used by st_get_strings() and st_set_strings() to ensure IDs and string data don't get mixed up.
*/
typedef struct {
        uint32_t id;
        char * data;
} st_string_data;

/*********************//**
    @name Return Codes
    @brief Error codes signify an issue that caused a function to exit prematurely. If a function exits prematurely, a reversal of any changes made during its execution is attempted before it exits.
*************************/
///@{

LIBSTRINGS extern const unsigned int LIBSTRINGS_OK;  ///< The function completed successfully.
LIBSTRINGS extern const unsigned int LIBSTRINGS_ERROR_INVALID_ARGS;  ///< Invalid arguments were given for the function.
LIBSTRINGS extern const unsigned int LIBSTRINGS_ERROR_NO_MEM;  ///< The library was unable to allocate the required memory.
LIBSTRINGS extern const unsigned int LIBSTRINGS_ERROR_FILE_WRITE_FAIL;  ///< A file could not be written to.
LIBSTRINGS extern const unsigned int LIBSTRINGS_ERROR_FILE_READ_FAIL;  ///< A file could not be read.
LIBSTRINGS extern const unsigned int LIBSTRINGS_ERROR_BAD_STRING;  ///< A string provided contains invalid byte sequences.

/**
    @brief Matches the value of the highest-numbered return code.
    @details Provided in case clients wish to incorporate additional return codes in their implementation and desire some method of avoiding value conflicts.
*/
LIBSTRINGS extern const unsigned int LIBSTRINGS_RETURN_MAX;

///@}


/**************************//**
    @name Version Functions
******************************/
///@{

/**
    @brief Checks for library compatibility.
    @details Checks whether the loaded libstrings is compatible with the given version of libstrings, abstracting library stability policy away from clients. The version numbering used is major.minor.patch.
    @param versionMajor The major version number to check.
    @param versionMinor The minor version number to check.
    @param versionPatch The patch version number to check.
    @returns True if the library versions are compatible, false otherwise.
*/
LIBSTRINGS bool st_is_compatible(const unsigned int versionMajor, const unsigned int versionMinor, const unsigned int versionPatch);

/**
    @brief Gets the library version.
    @details Outputs the major, minor and patch version numbers for the loaded libstrings. The version numbering used is major.minor.patch.
    @param versionMajor A pointer to the major version number.
    @param versionMinor A pointer to the minor version number.
    @param versionPatch A pointer to the patch version number.
*/
LIBSTRINGS void st_get_version(unsigned int * const versionMajor, unsigned int * const versionMinor, unsigned int * const versionPatch);

///@}

/*********************************//**
    @name Error Handling Functions
*************************************/
///@{

/**
   @brief Returns the message for the last error or warning encountered.
   @details Outputs a string giving the a message containing the details of the last error or warning encountered by a function. Each time this function is called, the memory for the previous message is freed, so only one error message is available at any one time.
   @param details A pointer to the error details string outputted by the function.
   @returns A return code.
*/
LIBSTRINGS unsigned int st_get_error_message(const char ** const details);

/**
   @brief Frees the memory allocated to the last error details string.
*/
LIBSTRINGS void st_cleanup();

///@}


/***************************************//**
    @name Lifecycle Management Functions
*******************************************/
///@{

/**
    @brief Initialise a new strings handle.
    @details Opens a STRINGS, ILSTRINGS or DLSTRINGS file, outputting a handle for the strings it contains. If the file doesn't exist then a handle for a new file will be created. You can create multiple handles.
    @param sh A pointer to the handle that is created by the function.
    @param path A string containing the relative or absolute path to the strings file to be opened. The file extension must be one of `.STRINGS`, `.DLSTRINGS` or `.ILSTRINGS`.
    @param fallbackEncoding The encoding that should be used to interpret any strings in the file that are not valid UTF-8 strings. Accepted values are `Windows-1250`, `Windows-1251` and `Windows-1252`.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_open(st_strings_handle * const sh, const char * const path, const char * const fallbackEncoding);

/**
    @brief Saves the strings associated with a handle.
    @details Saves the strings associated with the given handle to the given path, using the given encoding. Duplicate string entries are skipped, as are any unreferenced strings. If a file is loaded then saved by libstrings, the order of its contents may not match their order in the original file. This does not affect Skyrim's handling of the files, as the order does not matter.
    @param sh The handle the function acts on.
    @param path A string containing the relative or absolute path to the strings file to be saved to. The file extension must be one of `.STRINGS`, `.DLSTRINGS` or `.ILSTRINGS`.
    @param fallbackEncoding The encoding in which the strings should be written. Accepted values are `UTF-8`, `Windows-1250`, `Windows-1251` and `Windows-1252`.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_save(st_strings_handle sh, const char * const path, const char * const encoding);

/**
    @brief Closes an existing handle.
    @brief Closes an existing handle, freeing any memory allocated during its use.
    @param sh The handle to be destroyed.
*/
LIBSTRINGS void st_close(st_strings_handle sh);

///@}


/***************************************//**
    @name String Reading Functions
*******************************************/
///@{

/**
    @brief Gets an array of all strings with assigned IDs, that are associated with the given handle.
    @param sh The handle the function acts on.
    @param strings The outputted array of strings. If numStrings is `0`, this will be `NULL`.
    @param numStrings The size of the outputted array.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_get_strings(st_strings_handle sh, st_string_data ** const strings, size_t * const numStrings);

/**
    @brief Gets an array any strings that are associated with the given handle but lack IDs.
    @param sh The handle the function acts on.
    @param strings The outputted array of strings. If numStrings is `0`, this will be `NULL`.
    @param numStrings The size of the outputted array.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_get_unref_strings(st_strings_handle sh, char *** const strings, size_t * const numStrings);

/**
    @brief Gets the string with the given ID.
    @details Outputs the string with the given ID, associated with the given handle. If no string is found with that ID, the function returns an error code.
    @param sh The handle the function acts on.
    @param stringId The ID for which to return the associated string.
    @param string The outputted string. If no string with the given ID is found, this will be `NULL`.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_get_string(st_strings_handle sh, const uint32_t stringId, char ** const string);

///@}


/***************************************//**
    @name String Writing Functions
*******************************************/
///@{

/**
    @brief Replaces the strings associated with the given handle.
    @details Replaces all strings, excluding unreferenced strings, and IDs currently associated with the given handle with the given strings and IDs.
    @param sh The handle the function acts on.
    @param strings The inputted array of strings.
    @param numStrings The size of the `strings` array.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_set_strings(st_strings_handle sh, const st_string_data * const strings, const size_t numStrings);

/**
    @brief Add a string to the given handle.
    @details Adds the given string to the set of strings associated with the given handle, giving it the given ID.
    @param sh The handle the function acts on.
    @param stringId  The ID to be given to the added string. This ID must not already be present in the strings handle, or the function will return an error code.
    @param str The string to be added.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_add_string(st_strings_handle sh, const uint32_t stringId, const char * const str);

/**
    @brief Replaces a string associated with the given handle.
    @details Replaces the string associated with the given ID with the given string. If no string with that ID is found, the function returns an error code.
    @param sh The handle the function acts on.
    @param stringId  The ID of the string to be replaced.
    @param newString The replacement string.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_replace_string(st_strings_handle sh, const uint32_t stringId, const char * const newString);

/**
    @brief Removes the string with the given ID from the given handle.
    @details Removes the string associated with the given ID. If no string with that ID is found, the function returns an error code.
    @param sh The handle the function acts on.
    @param stringId The ID of the string to be removed.
    @returns A return code.
*/
LIBSTRINGS unsigned int st_remove_string(st_strings_handle sh, const uint32_t stringId);

///@}

#ifdef __cplusplus
}
#endif

#endif
