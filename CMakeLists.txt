cmake_minimum_required (VERSION 2.8)
project (libstrings)

# Set some vars.
set (libstrings_VERSION_MAJOR 1)
set (libstrings_VERSION_MINOR 0)
set (libstrings_VERSION_PATCH 0)

set (LIBSTR_SRC "${CMAKE_SOURCE_DIR}/src")
set (LIBSTR_LIBS boost_filesystem boost_system)
set (LIBSTR_LIB_SRC "${LIBSTR_SRC}/exception.cpp" "${LIBSTR_SRC}/format.cpp" "${LIBSTR_SRC}/helpers.cpp" "${LIBSTR_SRC}/libstrings.cpp")

# Set compiler flags.
set (CMAKE_C_FLAGS  "-m${LIBSTR_ARCH}  -static -static-libstdc++ -static-libgcc")
set (CMAKE_CXX_FLAGS "-m${LIBSTR_ARCH} -static -static-libstdc++ -static-libgcc")
set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

# Include source and library directories.
include_directories     ("${CMAKE_SOURCE_DIR}/externals/boost" "${CMAKE_SOURCE_DIR}/externals/utf8" ${LIBSTR_SRC})
# Depending on platform and architecture, the libs are stored in different places.
IF (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)
    link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-mingw-${LIBSTR_ARCH}/lib")
ELSE (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)
    link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-${LIBSTR_ARCH}/lib")
ENDIF (DEFINED LIBSTRINGS_CC_LINUX_WINDOWS)

# Build libstrings library.
add_library         (strings ${LIBSTR_LINK} ${LIBSTR_LIB_SRC})
target_link_libraries   (strings ${LIBSTR_LIBS})

# Build libstrings tester.
add_executable          (libstrings-tester ${LIBSTR_SRC}/tester.cpp)
target_link_libraries   (libstrings-tester strings)
