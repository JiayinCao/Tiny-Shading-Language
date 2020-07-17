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
set(all_files ${project_headers} ${project_cpps} ${project_cs} ${project_ccs} ${gtest_files})

# give each file a proper folder based on its path
source_group_by_dir(all_files)

# a separate folder for files outside this folder
source_group( "google test" FILES ${gtest_files} )

# include directories
include_directories( "${TSL_SOURCE_DIR}/src/thirdparty" ${LLVM_INCLUDE_DIR} )

# specify the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY            "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${TSL_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${TSL_SOURCE_DIR}/bin")

# an unit test project to verify features are ready in LLVM library
add_executable(LLVM_Test ${all_files})

# Setup correct output name for different configurations
set_target_properties( LLVM_Test PROPERTIES RELEASE_OUTPUT_NAME "llvm_test_r" )
set_target_properties( LLVM_Test PROPERTIES DEBUG_OUTPUT_NAME "llvm_test_d" )

# disable all warnings generated from LLVM
if (TSL_PLATFORM_WIN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4146 /wd4267 /wd4141 /wd4624 /wd4005 /wd4996" )

    # enable multi-thread compiling
    SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP${N}")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")

    set_target_properties( LLVM_Test PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} /MD /EHsc" )
endif()

if (TSL_PLATFORM_MAC OR TSL_PLATFORM_LINUX)
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# specify the llvm library to link
llvm_map_components_to_libnames(llvm_libs Core ExecutionEngine Interpreter MC MCJIT Support nativecodegen)

# link llvm libaries
target_link_libraries(LLVM_Test ${llvm_libs})