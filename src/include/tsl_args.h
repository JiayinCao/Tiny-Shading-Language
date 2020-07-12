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

#include <string>
#include <vector>
#include "tsl_version.h"

TSL_NAMESPACE_BEGIN

struct GlobalVar {
    std::string m_name;
    std::string m_type;

    GlobalVar(const std::string& name, const std::string& type) :
        m_name(name), m_type(type) {}
};

typedef std::vector<GlobalVar> GlobalVarList;

#define DECLARE_TSLGLOBAL_BEGIN()           struct TslGlobal {
#define DECLARE_TSLGLOBAL_VAR(VT,V)         VT V;
#define DECLARE_TSLGLOBAL_END()             static GlobalVarList m_offsets; static void RegisterGlobal( ShadingSystem& ); };

#define IMPLEMENT_TSLGLOBAL_BEGIN()         GlobalVarList TslGlobal::m_offsets({
#define IMPLEMENT_TSLGLOBAL_VAR(VT,V)       { GlobalVar( #V, #VT ) },
#define IMPLEMENT_TSLGLOBAL_END()           }); void TslGlobal::RegisterGlobal( ShadingSystem& ss ) { ss.register_tsl_global( m_offsets ); }

enum ShaderArgumentTypeEnum : unsigned int {
    TSL_TYPE_INVALID = 0,
    TSL_TYPE_INT,
    TSL_TYPE_FLOAT,
    TSL_TYPE_DOUBLE,
    TSL_TYPE_BOOL,
    TSL_TYPE_FLOAT3,
    TSL_TYPE_GLOBAL,
    TSL_TYPE_CLOSURE
};

struct float3 {
    float x, y, z;
};

inline float3 make_float3(float x, float y, float z) {
    float3 ret;
    ret.x = x; ret.y = y; ret.z = z;;
    return ret;
}

union ArgDefaultValue {
    float       m_float;
    int         m_int;
    double      m_double;
    bool        m_bool;
    float3      m_float3;
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

using ClosureID = int;
constexpr ClosureID INVALID_CLOSURE_ID = 0;
constexpr ClosureID CLOSURE_ADD = -1;
constexpr ClosureID CLOSURE_MUL = -2;

using ClosureParamPtr = void*;

struct ClosureTreeNodeBase {
    ClosureID           m_id = INVALID_CLOSURE_ID;
    ClosureParamPtr     m_params = nullptr;
};

struct ClosureTreeNodeAdd : public ClosureTreeNodeBase {
    ClosureTreeNodeBase* m_closure0 = nullptr;
    ClosureTreeNodeBase* m_closure1 = nullptr;
};

struct ClosureTreeNodeMul : public ClosureTreeNodeBase {
    float m_weight = 1.0f;
    ClosureTreeNodeBase* m_closure = nullptr;
};

// It is very important to make sure the memory layout is as expected, there should be no fancy stuff compiler tries to do for these data structure.
// Because the same data structure will also be generated from LLVM, which will expect this exact memory layout. If there is miss-match, it will crash.
static_assert(sizeof(ClosureTreeNodeBase) == sizeof(ClosureID) + sizeof(ClosureParamPtr) + 4 /* memory padding. */, "Invalid Closure Tree Node Size");
static_assert(sizeof(ClosureTreeNodeAdd) == sizeof(ClosureTreeNodeBase) + sizeof(ClosureTreeNodeBase*) * 2, "Invalid ClosureTreeNodeAdd Node Size");
static_assert(sizeof(ClosureTreeNodeMul) == sizeof(ClosureTreeNodeBase) + sizeof(float) + 4 /* memory padding. */ + sizeof(ClosureTreeNodeBase*), "Invalid ClosureTreeNodeMul Node Size");

struct ClosureTree {
    ClosureTreeNodeBase* m_root = nullptr;
};

struct ClosureVar {
    const std::string m_name;
    const std::string m_type;

    ClosureVar(const std::string& name, const std::string& type) :
        m_name(name), m_type(type) {}
};

typedef std::vector<ClosureVar> ClosureVarList;

#define DECLARE_CLOSURE_TYPE_BEGIN(T, name)     struct T { static const char* get_name() { return name; }
#define DECLARE_CLOSURE_TYPE_VAR(T,VT,V)        VT V;
#define DECLARE_CLOSURE_TYPE_END(T)             static ClosureVarList m_offsets; static ClosureID RegisterClosure( Tsl_Namespace::ShadingSystem& ); };

#define IMPLEMENT_CLOSURE_TYPE_BEGIN(T)         ClosureVarList T::m_offsets({
#define IMPLEMENT_CLOSURE_TYPE_VAR(T,VT,V)      { ClosureVar( #V, #VT ) },
#define IMPLEMENT_CLOSURE_TYPE_END(T)           }); ClosureID T::RegisterClosure( Tsl_Namespace::ShadingSystem& ss ) { return ss.register_closure_type( T::get_name() , m_offsets , sizeof(T) ); }

TSL_NAMESPACE_END