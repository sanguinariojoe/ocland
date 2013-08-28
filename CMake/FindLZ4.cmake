# Find liblz4
# LZ4_FOUND - system has the LZ4 library
# LZ4_INCLUDE_DIR - the LZ4 include directory
# LZ4_LIBRARIES - The libraries needed to use LZ4

if (LZ4_INCLUDE_DIR AND LZ4_LIBRARIES)
# in cache already
SET(LZ4_FOUND TRUE)
else (LZ4_INCLUDE_DIR AND LZ4_LIBRARIES)
FIND_PATH(LZ4_INCLUDE_DIR lz4.h
${LZ4_ROOT}/include/
/usr/include/
/usr/local/include/
/sw/lib/
/sw/local/lib/
)

if(WIN32 AND MSVC)
else(WIN32 AND MSVC)
FIND_LIBRARY(LZ4_LIBRARIES NAMES lz4
PATHS
${LZ4_ROOT}/lib
/usr/lib
/usr/local/lib
/sw/lib
/sw/local/lib
)
endif(WIN32 AND MSVC)

if (LZ4_INCLUDE_DIR AND LZ4_LIBRARIES)
set(LZ4_FOUND TRUE)
endif (LZ4_INCLUDE_DIR AND LZ4_LIBRARIES)

if (LZ4_FOUND)
if (NOT LZ4_FIND_QUIETLY)
message(STATUS "Found LZ4: ${LZ4_LIBRARIES}")
endif (NOT LZ4_FIND_QUIETLY)
else (LZ4_FOUND)
if (LZ4_FIND_REQUIRED)
message(FATAL_ERROR "Could NOT find LZ4")
endif (LZ4_FIND_REQUIRED)
endif (LZ4_FOUND)

MARK_AS_ADVANCED(LZ4_INCLUDE_DIR LZ4_LIBRARIES)
endif (LZ4_INCLUDE_DIR AND LZ4_LIBRARIES)
