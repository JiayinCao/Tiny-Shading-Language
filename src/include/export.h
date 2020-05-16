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

#include "platform.h"

#define TSL_EXPORTED

/*
#ifdef TSL_ON_WINDOWS
    #ifdef BUILDING_TSL
        #ifdef __GNUC__
            #define TSL_EXPORTED __attribute__ ((dllexport))
        #else
            #define TSL_EXPORTED __declspec(dllexport)
        #endif
    #else
        #ifdef __GNUC__
            #define TSL_EXPORTED __attribute__ ((dllimport))
        #else
            #define TSL_EXPORTED __declspec(dllimport)
        #endif
    #endif
#elif defined(TSL_ON_LINUX) || defined(TSL_ON_MAC)
    #define TSL_EXPORTED  __attribute__ ((visibility ("default")))
    //#define NOT_EXPORTED  __attribute__ ((visibility ("hidden")))
#else
    #define TSL_EXPORTED
#endif
*/