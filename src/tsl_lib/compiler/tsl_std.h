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

TSL_NAMESPACE_BEGIN

static const char* g_tsl_std = R"(

// This is the standard library of Tiny Shading Language.
// It defines some common data structure and global functions used in TSL shader units.

// Following data structures have to match exactly with what is defined in closures.h
// Any mismatch in memory layout will cause a crash.
struct closure_base {
	int		m_id;
	int*	m_params;
};

struct closure_mul {
	int		m_id;
	int*	m_params;
	float	m_weight;
	int*	m_closure;
};

struct closure_add {
	int		m_id;
	int*	m_params;
	int*	m_closure0;
	int*	m_closure1;
};

// Allocate memory
// TSL uses standard memory malloc for now, it will delegate this memory allocation to renderer later.
int*	malloc(int size);

struct float3{
	float x;
	float y;
	float z;
};

struct float4{
	float x;
	float y;
	float z;
	float w;
};

)";

TSL_NAMESPACE_END
