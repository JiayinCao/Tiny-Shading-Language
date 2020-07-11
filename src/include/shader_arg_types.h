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

#include "tsl_define.h"

TSL_NAMESPACE_BEGIN

enum ShaderArgumentTypeEnum : unsigned int {
    TSL_TYPE_INVALID = 0,
    TSL_TYPE_INT,
    TSL_TYPE_FLOAT,
    TSL_TYPE_DOUBLE,
    TSL_TYPE_BOOL,
    TSL_TYPE_FLOAT3,
    TSL_TYPE_FLOAT4,
    TSL_TYPE_GLOBAL,
    TSL_TYPE_CLOSURE
};

struct float3 {
    float x, y, z;
};

struct float4 {
    float x, y, z, w;
};

inline float3 make_float3(float x, float y, float z) {
    float3 ret;
    ret.x = x; ret.y = y; ret.z = z;;
    return ret;
}

inline float4 make_float4(float x, float y, float z, float w) {
    float4 ret;
    ret.x = x; ret.y = y; ret.z = z; ret.w = w;
    return ret;
}

union ArgDefaultValue {
    float       m_float;
    int         m_int;
    double      m_double;
    bool        m_bool;
    float3      m_float3;
    float4      m_float4;
    const char* m_global_var_name;  // to keep it as simple as possible, it is up to renderer to keep track of the memory
};

struct ArgDescriptor {
    std::string             m_name;
    ShaderArgumentTypeEnum  m_type = ShaderArgumentTypeEnum::TSL_TYPE_INVALID;
    bool                    m_is_output = false;
};

struct ShaderUnitInputDefaultValue {
    ShaderArgumentTypeEnum  m_type = ShaderArgumentTypeEnum::TSL_TYPE_INVALID;
    ArgDefaultValue         m_val;
};

using generic_ptr = int*;

TSL_NAMESPACE_END
