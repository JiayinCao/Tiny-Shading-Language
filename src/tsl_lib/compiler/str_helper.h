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

// There are a few places requesting storing strings and comparisons too. This is a nice helper 
// file hiding the details of string implementation. Basically, it does two things
//
//   - Each string is an unique pointer, which makes comparison of string a lot cheapper.
//   - String memory will be kepted in an internal container, not explicit memory management is needed.
//   - It is already sychronized so it is thread safe.

#include "tsl_version.h"

TSL_NAMESPACE_BEGIN

// create a unique string
const char* make_str_unique(const char*);

TSL_NAMESPACE_END