cmake_minimum_required (VERSION 2.8)
project (libstrings)

# Set some vars.
set (libstrings_VERSION_MAJOR 1)
set (libstrings_VERSION_MINOR 0)
set (libstrings_VERSION_PATCH 0)

set (LIBSTR_SRC "${CMAKE_SOURCE_DIR}/src")
set (LIBSTR_LIBS boost_filesystem boost_system)
set (LIBSTR_LIB_SRC "${LIBSTR_SRC}/exception.cpp" "${LIBSTR_SRC}/format.cpp" "${LIBSTR_SRC}/helpers.cpp" "${LIBSTR_SRC}/libstrings.cpp")

set (CMAKE_CXX_FLAGS "-static")

# Include source and library directories.
include_directories     ("${CMAKE_SOURCE_DIR}/externals/boost" "${CMAKE_SOURCE_DIR}/externals/utf8" ${LIBSTR_SRC})
# Depending on platform and architecture, and whether cross-compiling, the
# libs are stored in different places.
IF (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)
    link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-mingw-32/lib")
ELSE (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)
    link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-32/lib")
ENDIF (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)

# Build libstrings library.
add_library             (strings ${LIBSTR_LIB_SRC})
target_link_libraries   (strings ${LIBSTR_LIBS})

# Build libstrings tester.
add_executable          (libstrings-tester ${LIBSTR_SRC}/tester.cpp)
target_link_libraries   (libstrings-tester strings ${LIBSTR_LIBS})
