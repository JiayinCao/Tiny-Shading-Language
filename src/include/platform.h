/*
    This file is a part of Tiny-Shading-Language or TSL, an open-source cross
    platform programming shading language.

    Copyright (c) 2020-2020 by Jiayin Cao - All rights reserved.

    TSL is a free software written for educational purpose. Anyone can distribute
    or modify it under the the terms of the GNU General Public License Version 3 as
    published by the Free Software Foundation. However, there is NO warranty that
    all components are functional in a perfect manner. Without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along with
    this program. If not, see <http://www.gnu.org/licenses/gpl-3.0.html>.
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define TSL_ON_WINDOWS
#elif defined(__linux__)
    #define TSL_ON_LINUX

    // Make sure the compiler is C++14 compatible. Otherwise, make it clear that it is necessary to compile TSL in an error message.
    #if (__cplusplus < 201300L)
    #  error "TSL heavily uses features of C++14/11, please make sure you have a C++14 compatible compiler."
    #endif
#elif defined(__APPLE__)
    #define TSL_ON_MAC

        // Make sure the compiler is C++14 compatible. Otherwise, make it clear that it is necessary to compile TSL in an error message.
    #if (__cplusplus < 201300L)
    #  error "TSL heavily uses features of C++14/11, please make sure you have a C++14 compatible compiler."
    #endif
#endif