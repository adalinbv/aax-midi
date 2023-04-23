cmake_minimum_required(VERSION 3.2.0)
set(cmake_verbose_makefile TRUE)

if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)
include (CheckIncludeFiles)

project(AAX_MIDI C CXX)
set(PACKAGE_NAME "AeonWave-MIDI")
set(PACKAGE "aaxmidi")

option(WERROR "Treat compile warnings as errors"   OFF)
option(RMALLOC "Enable memory debugging functions" OFF)

if(WIN32)
  set(AEONWAVE_DIR aax)
else(WIN32)
  set(AEONWAVE_DIR aax)
endif(WIN32)


if(NOT LIBTYPE)
  set(LIBTYPE SHARED)
endif(NOT LIBTYPE)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(SIZEOF_SIZE_T 8)
else()
  set(SIZEOF_SIZE_T 4)
endif()

# read 'version' file into a variable (stripping any newlines or spaces)
FILE(READ version versionFile)
STRING(STRIP ${versionFile} AAXMIDI_VERSION)
set(VERSION AAXMIDI_VERSION)

# split version string into components, note CMAKE_MATCH_0 is the entire regexp match
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" CPACK_PACKAGE_VERSION ${AAXMIDI_VERSION})

set(AAX_MIDI_MAJOR_VERSION ${CMAKE_MATCH_1})
set(AAX_MIDI_MINOR_VERSION ${CMAKE_MATCH_2})
set(AAX_MIDI_MICRO_VERSION ${CMAKE_MATCH_3})
set(AAX_MIDI_PATCH_LEVEL ${DATE})

set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Utilities to show the capabilities of AeonWave")

# We have some custom .cmake scripts not in the official distribution.
set(CMAKE_MODULE_PATH "${AAX_MIDI_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

include(CPack_Modules)
include(CPack_Defaults)
include(GNUInstallDirs)
include(package)

if(WIN32)

else(WIN32)
  add_definitions(-D_REENTRANT -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=199309L)

  # DEBIAN
  set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1), libaax3-bin (>= 3.0.0), libaax3-data (>= 3.4.0)")

  # RPM
  set(CPACK_RPM_PACKAGE_LICENSE "BSD")
  set(CPACK_RPM_PACKAGE_GROUP "System/System/GUI/KDE")
  set(CPACK_RPM_PACKAGE_REQUIRES "libaax3-bin >= 3.0.0, libaax3-data >= 3.4.0")
endif(WIN32)

# ZIP
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${AAXMIDI_VERSION}" CACHE INTERNAL "tarball basename")

# Add definitions, compiler switches, etc.
add_definitions(-DHAVE_CONFIG_H=1)
if(WERROR)
  add_definitions(-Werror)
endif()

if(MSVC)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG /LD")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG /LD")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG /LD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG /LD")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)

    option(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" ON)
    if(FORCE_STATIC_VCRT)
        foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
            if(${flag_var} MATCHES "/MD")
                STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            endif()
        endforeach(flag_var)
    endif()
else(MSVC)
    if(NOT MINGW)
      if (GCC OR CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wl,--hash-style=both -Wpointer-arith -Wparentheses")
      endif(GCC OR CLANG)
    else()
       add_definitions(-U__STRICT_ANSI__)
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libstdc++" CACHE STRING
          "Flags used by MingW."
          FORCE)
       set(CMAKE_C_FLAGS "${C_FLAGS} -static -static-libgcc" CACHE STRING
          "Flags used by MingW."
          FORCE)
    endif()
 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" CACHE STRING
          "Flags used by al build types."
          FORCE)
    set(C_FLAGS "${C_FLAGS} -Os -Werror=format-security -std=c99 -Wall" CACHE STRING
        "Flags used by al build types."
        FORCE)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${C_FLAGS} -g -O2 -D_DEBUG" CACHE STRING
        "Flags used by the compiler during Release with Debug Info builds."
        FORCE)
    set(CMAKE_C_FLAGS_MINSIZEREL "${C_FLAGS} -Os -s -fomit-frame-pointer -DNDEBUG" CACHE STRING
        "Flags used by the compiler during release minsize builds."
        FORCE)
    set(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -Os -s -fomit-frame-pointer -DNDEBUG"
        CACHE STRING "Flags used by the compiler during release builds"
        FORCE)
    set(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -g3 -D_DEBUG" CACHE STRING
        "Flags used by the compiler during debug builds."
        FORCE)
endif(MSVC)

# Required libraries
find_package(AAX COMPONENTS aax REQUIRED)
##find_package(AeonWave COMPONENTS aax REQUIRED)
if((GCC OR CLANG) AND RMALLOC)
  set(C_FLAGS "${C_FLAGS} -fsanitize=address")
  set(CXX_FLAGS "${CXX_FLAGS} -fsanitize=address")
  set(LDFLAGS "${LD_FLAGS} -fsanitize=address")
  set(RMALLOC OFF)
  find_package(XML REQUIRED)
else((GCC OR CLANG) AND RMALLOC)
  find_package(XML REQUIRED)
endif((GCC OR CLANG) AND RMALLOC)

# Add definitions, compiler switches, etc.
include_directories("${AAX_MIDI_SOURCE_DIR}"
    include
    ${AAX_INCLUDE_DIR}
    "${AAX_MIDI_BINARY_DIR}/include"
)

check_function_exists(strlcpy HAVE_STRLCPY)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(time.h HAVE_TIME_H)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/cmake_config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")

install(FILES
        include/aax/midi.h
        include/aax/buffer_map.hpp
        include/aax/byte_stream.hpp
        DESTINATION "include/${AEONWAVE_DIR}"
        COMPONENT Headers
)

# Man pages
if(UNIX AND NOT WIN32)
  configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/admin/aaxplaymidi.1.in"
      "${CMAKE_CURRENT_BINARY_DIR}/aaxplaymidi.1")

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/aaxplaymidi.1
          DESTINATION "${CMAKE_INSTALL_PREFIX}/man/man1"
          COMPONENT Applications)
endif()

if(WIN32)
  set(KERNEL_LIBRARY Kernel32)
  set(EXTRA_LIBS "-lwinmm")
else(WIN32)
  set(EXTRA_LIBS "-lm")
endif(WIN32)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
)
link_directories(
  ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/midi
)

set(LIBMIDI aaxmidi)

add_subdirectory(base)
midi_subdirectory(src)
midi_subdirectory(midi)
midi_subdirectory(utils)

if(NOT SINGLE_PACKAGE)
  include (CPack)
endif(NOT SINGLE_PACKAGE)

message("Build type: ${CMAKE_BUILD_TYPE}")
message("C flags: ${CMAKE_C_FLAGS}")
message("C++ flags: ${CMAKE_CXX_FLAGS}")
message("Linker flags: ${CMAKE_SHARED_LINKER_FLAGS}")

# Build a static library
add_library(MIDIOBJ_LIB OBJECT ${SOURCES})
add_library(${LIBMIDI}-static STATIC $<TARGET_OBJECTS:MIDIOBJ_LIB>)

# Build a shared library
set(LIB_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
add_library(${LIBMIDI} SHARED $<TARGET_OBJECTS:MIDIOBJ_LIB>)
set_target_properties(${LIBMIDI}
  PROPERTIES DEFINE_SYMBOL MIDI_BUILD_LIBRARY
  VERSION ${LIB_VERSION}
  SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}
)
if(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")
  set_target_properties(${LIBMIDI} PROPERTIES PREFIX "")
else()
  set_target_properties(${LIBMIDI} PROPERTIES COMPILE_FLAGS "-shared")
endif()

target_link_libraries(${LIBMIDI}
  ${EXTRA_LIBS}
  ${XML_LIBRARY}
)

# Add an install target here
# Note Windows DLL are specified by RUNTIME
install(TARGETS ${LIBMIDI}
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT Libraries
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Headers
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" NAMELINK_SKIP COMPONENT Libraries
)
install(TARGETS ${LIBMIDI}-static
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT Libraries
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Headers
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" NAMELINK_SKIP COMPONENT Libraries
)

#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
#CONFIGURE_FILE(
#  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
#  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#  IMMEDIATE @ONLY)
#ADD_CUSTOM_TARGET(uninstall
#  "${CMAKE_COMMAND}" -P "${AAX_MIDI_BINARY_DIR}/cmake_uninstall.cmake")
