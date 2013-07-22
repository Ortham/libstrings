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

#include "format.h"
#include "libstrings.h"
#include "error.h"
#include "helpers.h"
#include "streams.h"
#include <cstdio>
#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace libstrings;

namespace fs = boost::filesystem;

_strings_handle_int::_strings_handle_int(const string& path, const string& fallbackEncoding) :
    extStringDataArr(NULL),
    extStringArr(NULL),
    extString(NULL),
    extStringDataArrSize(0),
    extStringArrSize(0) {

    bool isDotStrings;

    //Check extension.
    const string ext = fs::path(path).extension().string();
    if (boost::iequals(ext, ".strings"))
        isDotStrings = true;
    else if (boost::iequals(ext, ".ilstrings") || boost::iequals(ext, ".dlstrings"))
        isDotStrings = false;
    else
        throw error(LIBSTRINGS_ERROR_INVALID_ARGS, "File passed does not have a valid extension.");

    //If the file already exists, parse it.
    if (fs::exists(path)) {
        libstrings::ifstream in(fs::path(path), ios::binary);
        in.exceptions(ios::failbit | ios::badbit | ios::eofbit);  //Causes ifstream::failure to be thrown if problem is encountered.

        /*The data for each string is stored in two separate places.
        The directory holds all the IDs and offsets, and the data block
        holds all the strings at their offsets.
        Loop through the directory and for each entry, record the ID,
        look up the string using the offset and store that.
        Quickest to read whole file into memory, parse it from there
        then free that memory. Jumping around inside a file stream is
        a bit slower. */
        uint8_t * fileContent;
        uint32_t fileSize;

        //Get the file's length.
        in.seekg(0, ios::end);
        fileSize = in.tellg();

        //Allocate memory.
        try {
            fileContent = new uint8_t[fileSize];
        } catch (bad_alloc& e) {
            throw error(LIBSTRINGS_ERROR_NO_MEM, e.what());
        }

        //Read whole file into memory.
        in.seekg(0, ios::beg);
        in.read((char*)fileContent, fileSize);

        in.close();

        //Get number of directory entries.
        uint32_t dirCount = *reinterpret_cast<uint32_t*>(fileContent);

        uint32_t pos = sizeof(uint32_t) * 2;
        uint32_t startOfData = sizeof(uint32_t) * 2 * (dirCount + 1);
        boost::unordered_set<uint32_t> offsets;
        while (pos < startOfData) {
            string str;
            uint32_t id = *reinterpret_cast<uint32_t*>(fileContent + pos);
            uint32_t offset = *reinterpret_cast<uint32_t*>(fileContent + pos + sizeof(uint32_t));
            uint32_t strPos = startOfData + offset;
            if (!isDotStrings)
                strPos += sizeof(uint32_t);

            //Find position of null pointer.
            char * nptr = strchr((char*)(fileContent + strPos), '\0');
            if (nptr == NULL)
                throw error(LIBSTRINGS_ERROR_FILE_READ_FAIL, "Could not read contents of \"" + path + "\".");

            //Now set string, transcoding if necessary.
            str = ToUTF8(string((char*)(fileContent + strPos), nptr - (char*)(fileContent + strPos)), fallbackEncoding);

            data.insert(pair<uint32_t, string>(id, str));
            offsets.insert(offset);

            pos += 2 * sizeof(uint32_t);
        }

        //Now before we delete the contents, let's look for unreferenced strings.
        pos = startOfData;
        boost::unordered_set<uint32_t>::iterator endIt = offsets.end();
        while (pos < fileSize) {
            stringstream out;
            string str;

            if (isDotStrings)
                out << fileContent + pos;
            else
                out << fileContent + pos + sizeof(uint32_t);
            str = out.str();

            if (offsets.find(pos - startOfData) == endIt)
                unrefStrings.emplace(ToUTF8(str, fallbackEncoding));

            pos += str.length() + 1;
            if (!isDotStrings)
                pos += sizeof(uint32_t);
        }

        delete [] fileContent;
    }
}

_strings_handle_int::~_strings_handle_int() {
    if (extString != NULL)
        delete [] extString;

    if (extStringDataArr != NULL) {
        for (size_t i=0; i < extStringDataArrSize; i++)
            delete [] extStringDataArr[i].data;
        delete [] extStringDataArr;
    }

    if (extStringArr != NULL) {
        for (size_t i=0; i < extStringArrSize; i++)
            delete [] extStringArr[i];
        delete [] extStringArr;
    }
}

//Save file data to given path.
void _strings_handle_int::Save(const std::string& path, const std::string& encoding) {
    //Save everything in memory to the file.
    string directory;
    string strData;
    boost::unordered_map<string, uint32_t> hashmap;
    bool isDotStrings;

    //Check extension.
    const string ext = fs::path(path).extension().string();
    if (boost::iequals(ext, ".strings"))
        isDotStrings = true;
    else if (boost::iequals(ext, ".ilstrings") || boost::iequals(ext, ".dlstrings"))
        isDotStrings = false;
    else
        throw error(LIBSTRINGS_ERROR_INVALID_ARGS, "File passed does not have a valid extension.");

    //Output to buffers.
    for (boost::unordered_map<uint32_t, string>::iterator it=data.begin(), endIt=data.end(); it != endIt; ++it) {

        /* Search for this pair's string in the hashset.
            If present, use the offset in the hashmap for the directory entry's offset,
            and don't add the string again.
            Otherwise, add as normal. */

        boost::unordered_map<string, uint32_t>::iterator searchIt = hashmap.find(it->second);
        if (searchIt != hashmap.end()) {
            //Write directory data to its buffer.
            directory   += string((char*)&(it->first), sizeof(uint32_t))
                        +  string((char*)&(searchIt->second), sizeof(uint32_t));
        } else {
            uint32_t len = strData.length();

            //Write directory data to its buffer.
            directory   += string((char*)&(it->first), sizeof(uint32_t))
                        +  string((char*)&len, sizeof(uint32_t));

            //Write string data to its buffer, and increment the dataSize.
            string str = it->second + '\0';
            if (!isDotStrings) {
                uint32_t size = str.length();
                str = string((char*)&size, sizeof(uint32_t)) + str;
            }
            strData += FromUTF8(str, encoding);

            //Add to hashset to prevent it being written again.
            hashmap.insert(pair<string, uint32_t>(it->second, len));
        }


    }
    uint32_t count = data.size();
    uint32_t dataSize = strData.length();

    //Now write out everything.
    libstrings::ofstream out(fs::path(path), ios::binary | ios::trunc);
    if (!out.good())
        throw error(LIBSTRINGS_ERROR_FILE_WRITE_FAIL, "Could not write to \"" + path + "\".");
    out.write((char*)&count, sizeof(uint32_t));
    out.write((char*)&dataSize, sizeof(uint32_t));
    out.write((char*)directory.data(), directory.length());
    out.write((char*)strData.data(), strData.length());

    out.close();
}
