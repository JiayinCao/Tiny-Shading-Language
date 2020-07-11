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

#include "tslversion.h"

TSL_NAMESPACE_BEGIN

// Hide the constructors to prevent it from being constructed in a way that TSL doesn't expect.
// Note, TSL won't even implemented these functions because if TSL users try to invoke either of these calls, compiling will
// fail first and there is no way to see undefined symbol error at all.
#define TSL_HIDE_CONSTRUCTOR(T, ...)    private:\
                                        T(__VA_ARGS__);    /* Hide construcotr. */ \
                                        T(T&);             /* Hide copy constructor. */ \
                                        T(T&&);            /* Hide movable constructor. */

// Helper function to make class friendship. Code using TSL will not be interested in learning the friendship among the classes.
// So if it is not TSL library code including this file, it won't even generate any code with this macro.
#if BUILDING_TSL
    #define TSL_MAKE_CLASS_FRIEND(T)    friend class T;
    #define TSL_MAKE_STRUCT_FRIEND(T)   friend struct T;
#else
    #define TSL_MAKE_CLASS_FRIEND(T)    /* ignore me */
    #define TSL_MAKE_STRUCT_FRIEND(T)   /* ignore me */
#endif

TSL_NAMESPACE_END