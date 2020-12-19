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
set(all_project_files ${project_headers} ${project_cpps} ${project_cs} ${project_ccs})
set(all_files ${all_project_files} ${tsl_include} ${gtest_files})

# give each file a proper folder based on its path
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${all_project_files})

# a separate folder for files outside this folder
source_group( "tsl_include" FILES ${tsl_include} )
source_group( "google test" FILES ${gtest_files} )

# include directories
include_directories( "${TSL_SOURCE_DIR}/src/thirdparty" "${TSL_SOURCE_DIR}/src/include" )

# specify the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY            "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${TSL_SOURCE_DIR}/bin")

# CMake will strip the full rpath that is generated in building process during installation.
# In order to link the libraries succesfully, it is necessary to specify the relatively path where the share lib is in.
if(TSL_PLATFORM_LINUX)
    SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")
elseif(TSL_PLATFORM_MAC)
    SET(CMAKE_INSTALL_RPATH "@loader_path/../lib;@loader_path")
endif()

# unit test project to make sure TSL is functional properly
add_executable(TSL_Test ${all_files})

# Setup correct output name for different configurations
set_target_properties( TSL_Test PROPERTIES RELEASE_OUTPUT_NAME "tsl_test_r" )
set_target_properties( TSL_Test PROPERTIES DEBUG_OUTPUT_NAME "tsl_test_d" )

if (TSL_PLATFORM_MAC OR TSL_PLATFORM_LINUX)
   set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -pthread")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

   # link to tsl library
   target_link_libraries( TSL_Test TSL_Lib )
endif()

if (TSL_PLATFORM_WIN)
    set_target_properties( TSL_Test PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /MD /wd4251 /wd4530" )

    # enable multi-thread compiling
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP${N}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")

    # ignore "warning LNK4199: /DELAYLOAD:shell32.dll ignored; no imports found from shell32.dll"
    target_link_libraries( TSL_Test PRIVATE "-ignore:4199" TSL_Lib )
endif()

# This allows install command
install(TARGETS TSL_Test
        RUNTIME DESTINATION bin)