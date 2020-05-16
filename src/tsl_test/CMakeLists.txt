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

# collect all files in this library
file(GLOB_RECURSE project_headers *.h *.hpp)
file(GLOB_RECURSE project_cpps *.cpp)
file(GLOB_RECURSE project_cs *.c)
file(GLOB_RECURSE project_ccs *.cc)

# group all categories of files into one macro
set(all_files ${project_headers} ${project_cpps} ${project_cs} ${project_ccs} ${gtest_files} ${tsl_include})

# give each file a proper folder based on its path
source_group_by_dir(all_files)

# a separate folder for files outside this folder
source_group( "tsl_include" FILES ${tsl_include} )
source_group( "google test" FILES ${gtest_files} )

# include directories
include_directories( "${TSL_SOURCE_DIR}/src/thirdparty" "${TSL_SOURCE_DIR}/src/include" )

# specify the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY            "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${TSL_SOURCE_DIR}/bin")

# unit test project to make sure TSL is functional properly
add_executable(TSL_Test ${all_files})

# Setup correct output name for different configurations
set_target_properties( TSL_Test PROPERTIES RELEASE_OUTPUT_NAME "tsl_test_r" )
set_target_properties( TSL_Test PROPERTIES DEBUG_OUTPUT_NAME "tsl_test_d" )

if (TSL_PLATFORM_MAC OR TSL_PLATFORM_LINUX)
   set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread -O3")
endif()

if (TSL_PLATFORM_WIN)
    set_target_properties( TSL_Test PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /MD" )
endif()