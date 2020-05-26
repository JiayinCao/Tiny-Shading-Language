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

#include <memory>
#include <type_traits>
#include <stddef.h>
#include <vector>
#include <string>
#include "tslversion.h"

TSL_NAMESPACE_BEGIN

using ClosureID = int;
constexpr ClosureID INVALID_CLOSURE_ID = 0;
constexpr ClosureID CLOSURE_ADD = -1;
constexpr ClosureID CLOSURE_MUL = -2;

struct ClosureTreeNodeAdd;
struct ClosureTreeNodeMul;

using ClosureParamPtr = void*;

struct ClosureTreeNodeBase {
    ClosureID           m_id = INVALID_CLOSURE_ID;
    ClosureParamPtr     m_params = nullptr;

	ClosureTreeNodeAdd* as_add_node() {
		return reinterpret_cast<ClosureTreeNodeAdd*>(this);
	}

    ClosureTreeNodeMul* as_mul_node() {
        return reinterpret_cast<ClosureTreeNodeMul*>(this);
    }
};

struct ClosureTreeNodeAdd : public ClosureTreeNodeBase {
    ClosureTreeNodeBase*	m_closure0 = nullptr;
    ClosureTreeNodeBase*	m_closure1 = nullptr;
};

struct ClosureTreeNodeMul : public ClosureTreeNodeBase {
	float m_weight = 1.0f;
    ClosureTreeNodeBase*	m_closure = nullptr;
};

// It is very important to make sure the memory layout is as expected, there should be no fancy stuff compiler is trying to do for these data structure.
// Because the same data structure will also be generated from LLVM, which will expect this exact memory layout. If there is miss-match, it will crash.
static_assert( sizeof(ClosureTreeNodeBase) == sizeof(ClosureID) + sizeof(ClosureParamPtr) + 4 /* memory padding. */, "Invalid Closure Tree Node Size" );
static_assert( sizeof(ClosureTreeNodeAdd) == sizeof(ClosureTreeNodeBase) + sizeof(ClosureTreeNodeBase*) * 2 , "Invalid ClosureTreeNodeAdd Node Size" );
static_assert( sizeof(ClosureTreeNodeMul) == sizeof(ClosureTreeNodeBase) + sizeof(float) + 4 /* memory padding. */ + sizeof(ClosureTreeNodeBase*), "Invalid ClosureTreeNodeMul Node Size");

struct ClosureTree {
    ClosureTreeNodeBase*	m_root = nullptr;
};

struct ClosureVar {
    const std::string m_name;
    const std::string m_type;

    ClosureVar(const std::string& name, const std::string& type) :
        m_name(name), m_type(type) {}
};

typedef std::vector<ClosureVar> ClosureVarList;

#define DECLARE_CLOSURE_TYPE_BEGIN(T)           struct T {
#define DECLARE_CLOSURE_TYPE_VAR(T,VT,V)        VT V;
#define DECLARE_CLOSURE_TYPE_END(T)             static ClosureVarList m_offsets; static ClosureID RegisterClosure( const std::string& , ShadingSystem& ); };

#define IMPLEMENT_CLOSURE_TYPE_BEGIN(T)         ClosureVarList T::m_offsets({
#define IMPLEMENT_CLOSURE_TYPE_VAR(T,VT,V)      { ClosureVar( #V, #VT ) },
#define IMPLEMENT_CLOSURE_TYPE_END(T)            }); ClosureID T::RegisterClosure( const std::string& name , ShadingSystem& ss ) { return ss.register_closure_type( name , m_offsets , sizeof(T) ); }

TSL_NAMESPACE_END