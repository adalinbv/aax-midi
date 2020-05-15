# Locate libebur128 library
# This module defines
# EBUR128_LIBRARY, the name of the library to link against
# EBUR128_FOUND, if false, do not try to link
# EBUR128_INCLUDE_DIR, where to find header
#
set(EBUR128_FOUND FALSE)

# use pkg-config to get the directories and then use these values
# in the find_path() and find_library() calls
find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
  pkg_check_modules(_EBUR128 libebur128)
endif(PKG_CONFIG_FOUND)

find_path(EBUR128_INCLUDE_DIR ebur128.h
  HINTS
  PATH_SUFFIXES include
  PATHS
  ${_EBUR128_INCLUDE_DIRS}
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local/include
  /usr/include
  /sw/include
  /opt/local/include
  /opt/csw/include
  /opt/include
  /mingw
)

find_library(EBUR128_LIBRARY
  NAMES ebur128
  HINTS
  PATH_SUFFIXES lib64 lib
  PATHS
  ${_EBUR128_LIBRARY_DIRS}
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
  /mingw
)

if(EBUR128_LIBRARY)
  set(EBUR128_FOUND TRUE)
endif(EBUR128_LIBRARY)
