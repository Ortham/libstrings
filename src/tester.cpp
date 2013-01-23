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

#include <iostream>
#include <stdint.h>
#include <fstream>

using namespace std;

int main() {
    strings_handle sh;
    const char * path = "/media/oliver/6CF05918F058EA3A/Users/Oliver/Downloads/Strings/Skyrim_Japanese.STRINGS";
    const char * newPath = "/media/oliver/6CF05918F058EA3A/Users/Oliver/Downloads/Strings/Skyrim_Japanese.STRINGS";
    const char * testMessage = "This is a test message.";
    unsigned int ret;
    string_data * dataArr;
    size_t dataArrSize;
    char * str;
    const char * error;
    uint32_t id;
    char ** stringArr;
    size_t stringArrSize;

    std::ofstream out("libstrings-tester.txt");
    if (!out.good()){
        cout << "File could not be opened for reading.";
        return 1;
    }

    out << "Using path: " << path << endl;

    out << "TESTING OpenStringsFile(...)" << endl;
    ret = OpenStringsFile(&sh, path, "Windows-1252");
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "OpenStringsFile(...) failed! Return code: " << ret << endl;
    else
        out << '\t' << "OpenStringsFile(...) successful!" << endl;

    out << "TESTING GetStrings(...)" << endl;
    ret = GetStrings(sh, &dataArr, &dataArrSize);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "GetStrings(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "GetStrings(...) successful! Number of strings: " << dataArrSize << endl;
        out << '\t' << "ID" << '\t' << "String" << endl;
        for (size_t i=0; i < dataArrSize; i++) {
            out << '\t' << dataArr[i].id << '\t' << dataArr[i].data << endl;
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
    ret = GetUnreferencedStrings(sh, &stringArr, &stringArrSize);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "GetUnreferencedStrings(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "GetUnreferencedStrings(...) successful! Number of strings: " << stringArrSize << endl;
        for (size_t i=0; i < stringArrSize; i++) {
            out << '\t' << stringArr[i] << endl;
        }
    }

    out << "TESTING SetString(...)" << endl;
    ret = EditString(sh, id, testMessage);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "SetString(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "SetString(...) successful!"  << endl;
    }

    out << "TESTING GetString(...)" << endl;
    ret = GetString(sh, id, &str);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "GetString(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "GetString(...) successful!"  << endl;
        out << '\t' << "String fetched: " << str << endl;
    }

    out << "TESTING AddString(...)" << endl;
    ret = AddString(sh, 500000, testMessage);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "AddString(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "AddString(...) successful!"  << endl;
    }

    out << "TESTING RemoveString(...)" << endl;
    ret = RemoveString(sh, 500000);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "RemoveString(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "RemoveString(...) successful!"  << endl;
    }

    out << "TESTING GetString(...)" << endl;
    ret = GetString(sh, 500000, &str);
    if (ret != LIBSTRINGS_OK) {
        out << '\t' << "GetString(...) failed! Return code: " << ret << endl;
        ret = GetLastErrorDetails(&error);
        if (ret != LIBSTRINGS_OK)
            out << '\t' << "Failed to get error message." << endl;
        else
            out << '\t' << "Error message: " << error << endl;
    } else {
        out << '\t' << "GetString(...) successful!"  << endl;
        out << '\t' << "String fetched: " << str << endl;
    }

    out << "TESTING SetStrings(...)" << endl;
    ret = SetStrings(sh, dataArr, dataArrSize);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "SetStrings(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "SetStrings(...) successful!" << endl;
    }

    out << "TESTING GetStrings(...)" << endl;
    ret = GetStrings(sh, &dataArr, &dataArrSize);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "GetStrings(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "GetStrings(...) successful! Number of strings: " << dataArrSize << endl;
        out << '\t' << "ID" << '\t' << "String" << endl;
        for (size_t i=0; i < dataArrSize; i++) {
            out << '\t' << dataArr[i].id << '\t' << dataArr[i].data << endl;
        }
    }

    out << "TESTING SaveStringsFile(...)" << endl;
    ret = SaveStringsFile(sh, newPath);
    if (ret != LIBSTRINGS_OK)
        out << '\t' << "SaveStrings(...) failed! Return code: " << ret << endl;
    else {
        out << '\t' << "SaveStrings(...) successful!" << endl;
    }

    out << "TESTING CloseStringsFile(...)" << endl;
    CloseStringsFile(sh);

    out.close();
    return 0;
}
