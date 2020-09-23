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

# get the current directory
get_filename_component(TSL_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)

# tsl version
set( TSL_LIBRARY_VERSION_MAJOR 1 )
set( TSL_LIBRARY_VERSION_MINOR 0 )
set( TSL_LIBRARY_VERSION_PATCH 1 )
set( TSL_VERSION "1.0.1" )

# useful macros for tsl library
set( TSL_INCLUDE_DIR    ${TSL_INSTALL_PREFIX}/include )
set( TSL_LIBRARY_DIR    ${TSL_INSTALL_PREFIX}/lib )
set( TSL_RUNTIME_DIR    ${TSL_INSTALL_PREFIX}/bin )
set( TSL_LIBS           tsl_r )

# helper function to copy dll to destination folder, this is needed on Windows to copy tsl dll
function( tsl_runtime_copy dest_dir )
    file(GLOB tsl_runtime "${TSL_INSTALL_PREFIX}/bin/tsl_r.*")
    file(COPY ${tsl_runtime} DESTINATION ${dest_dir})
endfunction()
