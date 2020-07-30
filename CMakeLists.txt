#
#   This file is a part of Tiny-Shading-Language or TSL, an open-source cross
#   platform programming shading language.
#
#   Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.
#
#   TSL is a free software written for educational purpose. Anyone can distribute
#   or modify it under the the terms of the GNU General Public License Version 3 as
#   published by the Free Software Foundation. However, there is NO warranty that
#   all components are functional in a perfect manner. Without even the implied
#   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with
#   this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
#

cmake_minimum_required (VERSION 3.7)

# somehow using the default policy of 3.7 will crash the unit tests, I have no time looking into it for now
cmake_policy(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET( LLVM_DEBUG  "NO"   CACHE BOOL "Compile with debug version llvm." )

# this will avoid generate ZERO_CHECK project
# set(CMAKE_SUPPRESS_REGENERATION true)

# disallow generation of the unit test projects
OPTION(BUILD_TSL_INSTALL "Only generate TSL library" OFF)

# define platform
if (WIN32)
    set(TSL_PLATFORM_WIN true)
    set(TSL_PLATFORM_MAC false)
    set(TSL_PLATFORM_LINUX false)
    set(TSL_PLATFORM_NAME "Windows" )
elseif(APPLE)
    set(TSL_PLATFORM_WIN false)
    set(TSL_PLATFORM_MAC true)
    set(TSL_PLATFORM_LINUX false)
    set(TSL_PLATFORM_NAME "Mac OS" )
elseif(UNIX)
    set(TSL_PLATFORM_WIN false)
    set(TSL_PLATFORM_MAC false)
    set(TSL_PLATFORM_LINUX true)
    set(TSL_PLATFORM_NAME "Linux" )
endif()

set (PROJ_NAME ${PROJECT_NAME})
set (TSL_LIBRARY_VERSION_MAJOR 1)
set (TSL_LIBRARY_VERSION_MINOR 0)
set (TSL_LIBRARY_VERSION_PATCH 1)

# create TSL solution
project (TSL)

# generate tsl header file based on tsl version
set (versionfile tsl_version.h)
message (STATUS "Create ${versionfile} from ${versionfile}.in")
configure_file (${TSL_SOURCE_DIR}/src/include/${versionfile}.in "${TSL_SOURCE_DIR}/src/include/${versionfile}" @ONLY)
list (APPEND version_head "${TSL_SOURCE_DIR}/src/include/${versionfile}")

# loading llvm library, this is mandatory for compiling tsl, make sure it looks for a local llvm library
set(LLVM_DIR ${TSL_SOURCE_DIR}/dependencies/llvm/lib/cmake/llvm)

# make sure this new policy is enabled to surpress an unnecessary warning
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    cmake_policy(SET CMP0074 NEW)
endif()

# find llvm librar${TSL_SOURCE_DIR}/thirdparty/gtesty
find_package(LLVM REQUIRED CONFIG)
# output some information to make sure llvm is correctly located
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# google test is packed with tsl source code
set(GTEST_DIR ${TSL_SOURCE_DIR}/src/thirdparty/gtest)
MARK_AS_ADVANCED(GTEST_DIR)
# find google test library
find_package(GTEST REQUIRED CONFIG)

# this is all include files of TSL core library
file(GLOB_RECURSE tsl_include src/include/*.h)

# a common utility helper macro to group files
macro(source_group_by_dir source_files)
    set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
    foreach(sgbd_file ${${source_files}})
        string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
        string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
        string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
        string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
        if(sgbd_nogroup)
            set(sgbd_group_name "\\")
        endif(sgbd_nogroup)
        source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach(sgbd_file)
endmacro(source_group_by_dir)

# make sure there is a seperate macro for debug build
set( CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -DTSL_DEBUG")

set( CMAKE_CXX_FLAGS "${GTEST_HAS_TR1_TUPLE} -DGTEST_HAS_TR1_TUPLE=0" )

add_subdirectory( src/tsl_lib )
add_subdirectory( src/tsl_test )

# only generate the two projects if this macro is not defined
if (NOT ${BUILD_TSL_INSTALL})
	add_subdirectory( src/llvm_test )
	add_subdirectory( src/tsl_sample )
endif()

# hide the llvm generated project
set_target_properties (intrinsics_gen PROPERTIES FOLDER LLVM_Generated)