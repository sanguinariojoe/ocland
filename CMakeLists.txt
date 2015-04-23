# ===================================================== #
#                                                       #
#    #    ##   #  #   #                           #     #
#   # #  #  #  #  #  # #                          #     #
#  ##### #  #  #  # #####  ##  ###  #  #  ## ###  ###   #
#  #   # #  #  #  # #   # #  # #  # #  # #   #  # #  #  #
#  #   # #  #  #  # #   # #  # #  # #  #   # #  # #  #  #
#  #   #  ## #  ##  #   #  ### ###   ### ##  ###  #  #  #
#                            # #             #          #
#                          ##  #             #          #
#                                                       #
# ===================================================== #
#                                                       #
#   Authors:                                            #
#       - Cercos Pita, Jose Luis                        #
#       - Miguel Gonzalez, Leo                          #
#       - Souto Iglesias, Antonio                       #
#                                                       #
# ===================================================== #


# ===================================================== #
# General CMake options                                 #
# ===================================================== #
cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(ocland C)
set(AQUAGPUSPH_VERSION "0.1.02")

set(PACKAGE_NAME "ocland")
set(PACKAGE_VERSION_MAJOR "0")
set(PACKAGE_VERSION_MINOR "2")
set(PACKAGE_VERSION_PATCH "03")
set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
enable_testing()

# ===================================================== #
# Packaging with CPack                                  #
# ===================================================== #
SET(CPACK_SET_DESTDIR ON) # Solve packaging files with absolute destination like one from /et/OpenCL/vendors
SET(CPACK_PACKAGE_CONTACT "Timur Magomedov <tim239@yandex.ru>")
IF(UNIX)
	SET(CPACK_GENERATOR "TBZ2;DEB")
	SET(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenCL over network implementation")
	SET(PACK_DEBIAN_PACKAGE_DESCRIPTION
		"ocland (OpenCL Land) is a cloud computing OpenCL interface. ocland will "
		"connect to the servers that you specify getting the OpenCL devices "
		"available, allowing you to perform computations along the network "
		"without changing your code!")
	STRING(REPLACE ";" "" PACK_DEBIAN_PACKAGE_DESCRIPTION "${PACK_DEBIAN_PACKAGE_DESCRIPTION}")
	SET(CPACK_STRIP_FILES ON)
	EXECUTE_PROCESS(COMMAND "dpkg --print-architecture" OUTPUT_VARIABLE  CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")#TODO add ", snappy" or other compression library if enabled
	SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
	SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
ELSE()
	SET(CPACK_GENERATOR "TBZ2;ZIP")
	FIND_PROGRAM(NSIS_PROGRAM makensis.exe
		PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\NSIS;InstallLocation]"
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\NSIS;InstallLocation]")
	if(NSIS_PROGRAM)
		SET(CPACK_GENERATOR "${CPACK_GENERATOR};NSIS")
	endif()
ENDIF()

INCLUDE(CPack)

# include local modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# No Debug/Release output paths
set(DEBUG_MAIN_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(RELEASE_MAIN_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
	set(PLATFORM_CP xcopy /Y /S)
	set(PLATFORM_MK mkdir)
else(WIN32)
	set(PLATFORM_CP cp)
	set(PLATFORM_MK mkdir -p)
endif(WIN32)

# ===================================================== #
# Output directories for install target                 #
# ===================================================== #
if(WIN32)
	SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Installation root directory")
else(WIN32)
	SET(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation root directory")
	SET(INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/bin")
endif(WIN32)

SET(CMAKE_INSTALL_DATADIR share/ocland CACHE PATH "Output directory for data and resource files")

MESSAGE(STATUS "prefix: ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "datadir: ${CMAKE_INSTALL_DATADIR}")

# ===================================================== #
# All the options for the build process                 #
# ===================================================== #
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)
OPTION(OCLAND_SERVER "Build and install ocland server." ON)
OPTION(OCLAND_SERVER_DAEMON "Build and install ocland server daemon." ON)
OPTION(OCLAND_SERVER_VERBOSE "Show the ocland server called methods." OFF)
OPTION(OCLAND_CLIENT "Build and install ocland client." ON)
OPTION(OCLAND_CLIENT_ICD "Update OpenCL drivers with the ocland one." ON)
OPTION(OCLAND_CLIENT_VERBOSE "Show the ICD called methods." OFF)
OPTION(OCLAND_EXAMPLES "Build ocland examples." ON)
OPTION(OCLAND_USE_COMPRESSION "Compress the large data prior to send it." ON)

IF(NOT DEFINED OCLAND_PORT)
	SET(OCLAND_PORT 51000 CACHE STRING "Port used by ocland for the main connection")
ENDIF(NOT DEFINED OCLAND_PORT)
IF(NOT DEFINED OCLAND_PORT_FIRST_ASYNC)
	SET(OCLAND_PORT_FIRST_ASYNC 51001 CACHE STRING "First port used to perform asynchronous data transfers")
ENDIF(NOT DEFINED OCLAND_PORT_FIRST_ASYNC)
IF(NOT DEFINED OCLAND_PORT_LAST_ASYNC)
	SET(OCLAND_PORT_LAST_ASYNC 51150 CACHE STRING "Last port used to perform asynchronous data transfers")
ENDIF(NOT DEFINED OCLAND_PORT_LAST_ASYNC)
IF(NOT DEFINED OCLAND_MAX_CLIENTS)
	SET(OCLAND_MAX_CLIENTS 32 CACHE STRING "Maximum number of clients that can be simultaneously connected to the server")
ENDIF(NOT DEFINED OCLAND_MAX_CLIENTS)
IF(NOT DEFINED OCLAND_COMPRESSION_LIBRARY)
	SET(OCLAND_COMPRESSION_LIBRARY SNAPPY CACHE STRING "Library to compress/uncompress data. Alternatives are ZLIB,LZO,LZMA,BZIP2,LZ4,SNAPPY")
ENDIF(NOT DEFINED OCLAND_COMPRESSION_LIBRARY)


MARK_AS_ADVANCED(OCLAND_PORT)
MARK_AS_ADVANCED(OCLAND_PORT_FIRST_ASYNC)
MARK_AS_ADVANCED(OCLAND_PORT_LAST_ASYNC)
MARK_AS_ADVANCED(OCLAND_MAX_CLIENTS)
MARK_AS_ADVANCED(OCLAND_COMPRESSION_LIBRARY)

# Ensure that ports provided are rightly defined
IF(OCLAND_PORT_FIRST_ASYNC STRGREATER OCLAND_PORT_LAST_ASYNC)
MESSAGE(FATAL_ERROR "Last port for asynchronous data transfers can't be lower than the first one!")
ENDIF(OCLAND_PORT_FIRST_ASYNC STRGREATER OCLAND_PORT_LAST_ASYNC)
IF((OCLAND_PORT_LAST_ASYNC STRLESS OCLAND_PORT) AND (OCLAND_PORT_FIRST_ASYNC STRGREATER OCLAND_PORT))
MESSAGE(FATAL_ERROR "Main connection port is included in the asynchronous data transfer ports!")
ENDIF((OCLAND_PORT_LAST_ASYNC STRLESS OCLAND_PORT) AND (OCLAND_PORT_FIRST_ASYNC STRGREATER OCLAND_PORT))
IF(OCLAND_PORT_FIRST_ASYNC STREQUAL OCLAND_PORT_LAST_ASYNC)
MESSAGE(WARNING "Only one port available for asynchronous data transfers!")
ENDIF(OCLAND_PORT_FIRST_ASYNC STREQUAL OCLAND_PORT_LAST_ASYNC)


# ===================================================== #
# Definitions                                           #
# ===================================================== #
ADD_DEFINITIONS(-DOCLAND_PORT=${OCLAND_PORT}
-DMAX_CLIENTS=${OCLAND_MAX_CLIENTS}
-DOCLAND_ASYNC_FIRST_PORT=${OCLAND_PORT_FIRST_ASYNC}
-DOCLAND_ASYNC_LAST_PORT=${OCLAND_PORT_LAST_ASYNC}
)
IF(OCLAND_CLIENT_VERBOSE)
ADD_DEFINITIONS(-DOCLAND_CLIENT_VERBOSE)
ENDIF(OCLAND_CLIENT_VERBOSE)
IF(OCLAND_SERVER_VERBOSE)
ADD_DEFINITIONS(-DOCLAND_SERVER_VERBOSE)
ENDIF(OCLAND_SERVER_VERBOSE)
# ===================================================== #
# Search the packages                                   #
# ===================================================== #

# OpenCL
FIND_PACKAGE(OpenCL REQUIRED)

IF(NOT OPENCL_FOUND)
MESSAGE(FATAL_ERROR "OpenCL not found, but ${PACKAGE_NAME} requires it. Please install OpenCL!")
ENDIF(NOT OPENCL_FOUND)

IF(NOT "${OPENCL_VERSION}" STREQUAL "")
	IF(OPENCL_VERSION_MAJOR LESS 1)
	MESSAGE(FATAL_ERROR "OpenCL ${OPENCL_VERSION} found, but OpenCL >= 1.2 is required. Please install an ICD loader!")
	ELSE(OPENCL_VERSION_MAJOR LESS 1)
		IF(OPENCL_VERSION_MAJOR EQUAL 1 AND OPENCL_VERSION_MINOR LESS 2)
		MESSAGE(FATAL_ERROR "OpenCL ${OPENCL_VERSION} found, but OpenCL >= 1.2 is required. Please install an ICD loader!")
		ENDIF(OPENCL_VERSION_MAJOR EQUAL 1 AND OPENCL_VERSION_MINOR LESS 2)
	ENDIF(OPENCL_VERSION_MAJOR LESS 1)
	MESSAGE(STATUS "OpenCL: ${OPENCL_VERSION}")
ELSE(NOT "${OPENCL_VERSION}" STREQUAL "")
	MESSAGE(STATUS "OpenCL: yes (version undefined)")
ENDIF(NOT "${OPENCL_VERSION}" STREQUAL "")

IF(NOT "${OPENCL_PLATFORM_VERSION}" STREQUAL "")
	IF(OPENCL_PLATFORM_VERSION_MAJOR LESS 1)
	MESSAGE(WARNING "OpenCL ${OPENCL_PLATFORM_VERSION} platform found, but OpenCL >= 1.2 is strongly recommended")
	ELSE(OPENCL_PLATFORM_VERSION_MAJOR LESS 1)
		IF(OPENCL_PLATFORM_VERSION_MAJOR EQUAL 1 AND OPENCL_PLATFORM_VERSION_MINOR LESS 2)
		MESSAGE(WARNING "OpenCL ${OPENCL_PLATFORM_VERSION} platform found, but OpenCL >= 1.2 is strongly recommended")
		ENDIF(OPENCL_PLATFORM_VERSION_MAJOR EQUAL 1 AND OPENCL_PLATFORM_VERSION_MINOR LESS 2)
	ENDIF(OPENCL_PLATFORM_VERSION_MAJOR LESS 1)
ENDIF(NOT "${OPENCL_PLATFORM_VERSION}" STREQUAL "")

# pthreads
IF(UNIX)
	# use cmake to find pthreads
	FIND_PACKAGE(Threads REQUIRED)
ELSEIF(WIN32)
	# use pthreads-win32 static library from submodule
	SET(PTHREADS_BUILD_STATIC ON)
	SET(PTHREADS_EXPORT ON)
	ADD_DEFINITIONS(-DPTW32_STATIC_LIB)
	ADD_SUBDIRECTORY(pthreads-win32)
	SET(THREADS_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pthreads-win32)
	SET(CMAKE_THREAD_LIBS_INIT pthreads)
	SET(Threads_FOUND TRUE)
ELSE()
	MESSAGE(WARNING "Unknown platform, trying to find POSIX threads library")
	FIND_PACKAGE(Threads REQUIRED)
ENDIF()

IF(NOT Threads_FOUND)
MESSAGE(FATAL_ERROR "pthreads not found, but ${PACKAGE_NAME} requires it. Please install pthreads!")
ENDIF(NOT Threads_FOUND)

IF(OCLAND_USE_COMPRESSION)
	# zlib
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "ZLIB")
		FIND_PACKAGE(ZLIB REQUIRED)

		IF(NOT ZLIB_FOUND)
		MESSAGE(FATAL_ERROR "zlib not found, but OCLAND_COMPRESSION_LIBRARY is ZLIB. Please install zlib or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT ZLIB_FOUND)

		ADD_DEFINITIONS(-DHAVE_ZLIB)
		SET(COMPRESSION_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS})
		SET(COMPRESSION_LIBRARIES ${ZLIB_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "ZLIB")

	# lzo
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZO")
		FIND_PACKAGE(LZO REQUIRED)

		IF(NOT LZO_FOUND)
		MESSAGE(FATAL_ERROR "lzo not found, but OCLAND_COMPRESSION_LIBRARY is LZO. Please install lzo or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT LZO_FOUND)

		ADD_DEFINITIONS(-DHAVE_LZO)
		SET(COMPRESSION_INCLUDE_DIRS ${LZO_INCLUDE_DIR})
		SET(COMPRESSION_LIBRARIES ${LZO_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZO")

	# lzma
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZMA")
		FIND_PACKAGE(LZMA REQUIRED)

		IF(NOT LZMA_FOUND)
		MESSAGE(FATAL_ERROR "lzma not found, but OCLAND_COMPRESSION_LIBRARY is LZMA. Please install lzma or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT LZMA_FOUND)

		ADD_DEFINITIONS(-DHAVE_LZMA)
		SET(COMPRESSION_INCLUDE_DIRS ${LZMA_INCLUDE_DIR})
		SET(COMPRESSION_LIBRARIES ${LZMA_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZMA")

	# bzip2
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "BZIP2")
		FIND_PACKAGE(BZip2 REQUIRED)

		IF(NOT BZIP2_FOUND)
		MESSAGE(FATAL_ERROR "bzip2 not found, but OCLAND_COMPRESSION_LIBRARY is BZIP2. Please install bzip2 or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT BZIP2_FOUND)

		ADD_DEFINITIONS(-DHAVE_BZIP2)
		SET(COMPRESSION_INCLUDE_DIRS ${BZIP2_INCLUDE_DIRS})
		SET(COMPRESSION_LIBRARIES ${BZIP2_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "BZIP2")

	# lz4
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZ4")
		FIND_PACKAGE(LZ4 REQUIRED)

		IF(NOT LZ4_FOUND)
		MESSAGE(FATAL_ERROR "lz4 not found, but OCLAND_COMPRESSION_LIBRARY is LZ4. Please install lz4 or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT LZ4_FOUND)

		ADD_DEFINITIONS(-DHAVE_LZ4)
		SET(COMPRESSION_INCLUDE_DIRS ${LZ4_INCLUDE_DIR})
		SET(COMPRESSION_LIBRARIES ${LZ4_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "LZ4")

	# lz4
	IF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "SNAPPY")
		FIND_PACKAGE(SNAPPY REQUIRED)

		IF(NOT SNAPPY_FOUND)
		MESSAGE(FATAL_ERROR "Snappy not found, but OCLAND_COMPRESSION_LIBRARY is SNAPPY. Please install Snappy or change OCLAND_COMPRESSION_LIBRARY")
		ENDIF(NOT SNAPPY_FOUND)

		ADD_DEFINITIONS(-DHAVE_SNAPPY)
		SET(COMPRESSION_INCLUDE_DIRS ${SNAPPY_INCLUDE_DIR})
		SET(COMPRESSION_LIBRARIES ${SNAPPY_LIBRARIES})
	ENDIF("${OCLAND_COMPRESSION_LIBRARY}" STREQUAL "SNAPPY")

ELSE(OCLAND_USE_COMPRESSION)
	ADD_DEFINITIONS(-DHAVE_NO_COMPRESSION)
ENDIF(OCLAND_USE_COMPRESSION)

IF(OCLAND_SERVER_DAEMON AND NOT OCLAND_SERVER)
	MESSAGE(SEND_ERROR "Server daemon requested, but server will not be built. Disable the deamon OCLAND_SERVER_DAEMON of activate the server OCLAND_SERVER")
ENDIF(OCLAND_SERVER_DAEMON AND NOT OCLAND_SERVER)

IF(OCLAND_CLIENT_ICD AND NOT OCLAND_CLIENT)
	MESSAGE(SEND_ERROR "OpenCL driver vendor requested, but client will not be built. Disable the icd OCLAND_CLIENT_ICD of activate the client OCLAND_CLIENT")
ENDIF(OCLAND_CLIENT_ICD AND NOT OCLAND_CLIENT)


# ===================================================== #
# Global Compiler and Linker Settings                   #
# ===================================================== #
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/include)

# check for 64-bit platform
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	MESSAGE(STATUS "Platform is 64-bit")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
	MESSAGE(STATUS "Platform is 32-bit")
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

IF(MSVC)
	# set default compiler settings
	#SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GF /GY")
	#SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DAQUA_DEBUG -DHAVE_GPUPROFILE")
	# set default libs
	#SET (CMAKE_C_STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib SHFolder.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib ")
	#set (CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES}")
	# set linker flag /nodefaultlib
	#set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB")
	#SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB")

	# Mark 32 bit executables large address aware so they can use > 2GB address space
	# NOTE: This setting only has an effect on machines with at least 3GB of RAM, although it sets the linker option it doesn't set the the linker switch 'Enable Large Addresses'
	IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
		SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
	ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 4)
ELSE(MSVC)
	SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DOCLAND_DEBUG")
	MESSAGE(STATUS "DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
ENDIF(MSVC)
IF(MINGW)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mthreads")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mthreads -Wl,--export-all-symbols")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -mthreads -Wl,--export-all-symbols")
	LINK_LIBRARIES(-lgdi32)
ENDIF(MINGW)
# 0000661: cmake build on Mac OS: dealing with dylib versus so
IF(APPLE)
	SET(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
ENDIF(APPLE)
IF(UNIX)
	ADD_DEFINITIONS(-DUNIX)
	IF(CMAKE_COMPILER_IS_GNUCC)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	ENDIF(CMAKE_COMPILER_IS_GNUCC)
ENDIF(UNIX)
IF(WIN32)
	ADD_DEFINITIONS(-DWIN32)
ENDIF(WIN32)
IF(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# ===================================================== #
# Compilation parts                                     #
# ===================================================== #
ADD_SUBDIRECTORY(src)

# ===================================================== #
# Show a brief                                          #
# ===================================================== #
MESSAGE(STATUS "=====================================================")
IF(OCLAND_SERVER)
	MESSAGE(STATUS "ocland server:")
	IF(OCLAND_SERVER_DAEMON)
		MESSAGE(STATUS "    - With daemon")
	ENDIF(OCLAND_SERVER_DAEMON)
	MESSAGE(STATUS "    - Listening in port ${OCLAND_PORT}")
	MESSAGE(STATUS "    - ${OCLAND_MAX_CLIENTS} clients will be accepted")
ENDIF(OCLAND_SERVER)
IF(OCLAND_CLIENT)
	MESSAGE(STATUS "ocland client:")
	IF(OCLAND_CLIENT_ICD)
		MESSAGE(STATUS "    - Updating OpenCL drivers list")
	ENDIF(OCLAND_CLIENT_ICD)
	MESSAGE(STATUS "    - Connecting to port ${OCLAND_PORT}")
ENDIF(OCLAND_CLIENT)
IF(OCLAND_USE_COMPRESSION)
	MESSAGE(STATUS "Data compression library: ${OCLAND_COMPRESSION_LIBRARY}")
ELSE(OCLAND_USE_COMPRESSION)
	MESSAGE(STATUS "Uncompressed data transfers")
ENDIF(OCLAND_USE_COMPRESSION)
IF(OCLAND_EXAMPLES)
	MESSAGE(STATUS "examples will be built")
ENDIF(OCLAND_EXAMPLES)
MESSAGE(STATUS "Destination: ${CMAKE_INSTALL_PREFIX}")
MESSAGE(STATUS "Data destination: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}")
MESSAGE(STATUS "=====================================================")
