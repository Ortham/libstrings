##############################
# General Settings
##############################

cmake_minimum_required (VERSION 2.8)
project (libstrings)

# Set some vars.
set (libstrings_VERSION_MAJOR 1)
set (libstrings_VERSION_MINOR 0)
set (libstrings_VERSION_PATCH 0)

set (LIBSTR_SRC "${CMAKE_SOURCE_DIR}/src")
set (LIBSTR_LIB_SRC "${LIBSTR_SRC}/exception.cpp" "${LIBSTR_SRC}/format.cpp" "${LIBSTR_SRC}/helpers.cpp" "${LIBSTR_SRC}/libstrings.cpp")

# Include source and library directories.
include_directories     ("${CMAKE_SOURCE_DIR}/externals/boost" "${CMAKE_SOURCE_DIR}/externals/utf8" ${LIBSTR_SRC})

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

# Settings when compiling and cross-compiling on Linux.
IF (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set (LIBSTR_LIBS boost_filesystem boost_system)
    set (CMAKE_C_FLAGS  "-m${LIBSTR_ARCH}  -static -static-libstdc++ -static-libgcc")
    set (CMAKE_CXX_FLAGS "-m${LIBSTR_ARCH} -static -static-libstdc++ -static-libgcc /EHsc")
    set (CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set (CMAKE_MODULE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")

    IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-mingw-${LIBSTR_ARCH}/lib")
        IF (LIBSTR_ARCH MATCHES "32")
            set (MINGW i586-mingw32msvc)
        ELSE ()
            set (MINGW x86_64-w64-mingw32)
        ENDIF ()

        set (CMAKE_C_COMPILER   ${MINGW}-gcc)
        set (CMAKE_CXX_COMPILER ${MINGW}-g++)
        set (CMAKE_RC_COMPILER  ${MINGW}-windres)
        set (CMAKE_RANLIB       ${MINGW}-ranlib)
    ENDIF ()
ENDIF ()

# Settings when not cross-compiling.
IF (CMAKE_SYSTEM_NAME MATCHES CMAKE_HOST_SYSTEM_NAME)
    link_directories    ("${CMAKE_SOURCE_DIR}/externals/boost/stage-${LIBSTR_ARCH}/lib")
ENDIF ()

##############################
# Actual Building
##############################

# Build libstrings library.
add_library         (strings ${LIBSTR_LINK} ${LIBSTR_LIB_SRC})
target_link_libraries   (strings ${LIBSTR_LIBS})

# Build libstrings tester.
add_executable          (libstrings-tester ${LIBSTR_SRC}/tester.cpp)
target_link_libraries   (libstrings-tester strings)
