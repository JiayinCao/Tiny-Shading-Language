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

#include "tsl_version.h"

TSL_NAMESPACE_BEGIN

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
    #define TSL_X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
    #define TSL_X64_TARGET
#endif

TSL_NAMESPACE_END