# Settings passed on the command line:
#
# LIBSTR_LIBS_DIR = the directory which all external libraries may be referenced from.
# LIBSTR_ARCH = the build architecture
# LIBSTR_LINK = whether to build a static or dynamic library.

##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8.9)
project (libstrings)

set (LIBSTR_SRC "${CMAKE_SOURCE_DIR}/src/exception.cpp" "${CMAKE_SOURCE_DIR}/src/format.cpp" "${CMAKE_SOURCE_DIR}/src/helpers.cpp" "${CMAKE_SOURCE_DIR}/src/libstrings.cpp")

# Include source and library directories.
include_directories ("${LIBSTR_LIBS_DIR}/boost" "${LIBSTR_LIBS_DIR}/utf8" "${CMAKE_SOURCE_DIR}/src")

##############################
# Platform-Specific Settings
##############################

# Settings when compiling for Windows.
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    IF (${LIBSTR_LINK} MATCHES "STATIC")
        add_definitions (-DLIBSTRINGS_STATIC)
    ELSE ()
        add_definitions (-DLIBSTRINGS_EXPORT)
    ENDIF ()
ENDIF ()

# Settings when compiling on Windows.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set (LIBSTR_LIBS libboost_filesystem-vc110-mt-1_52 libboost_system-vc110-mt-1_52)
    set (CMAKE_CXX_FLAGS "/EHsc")
ENDIF ()

# Settings when compiling and cross-compiling on Linux.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set (LIBSTR_LIBS boost_filesystem boost_system)
    set (CMAKE_C_FLAGS  "-m${LIBSTR_ARCH}")
    set (CMAKE_CXX_FLAGS "-m${LIBSTR_ARCH}")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        link_directories ("${LIBSTR_LIBS_DIR}/boost/stage-mingw-${LIBSTR_ARCH}/lib")
    ENDIF ()
ENDIF ()

# Settings when not cross-compiling.
IF (CMAKE_SYSTEM_NAME MATCHES CMAKE_HOST_SYSTEM_NAME)
    link_directories ("${LIBSTR_LIBS_DIR}/boost/stage-${LIBSTR_ARCH}/lib")
ENDIF ()

##############################
# Actual Building
##############################

# Build libstrings library.
add_library           (strings ${LIBSTR_LINK} ${LIBSTR_SRC})
target_link_libraries (strings ${LIBSTR_LIBS})

# Build libstrings tester.
add_executable        (libstrings-tester "${CMAKE_SOURCE_DIR}/src/tester.cpp")
target_link_libraries (libstrings-tester strings ${LIBSTR_LIBS})
