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

// Data types that can be used as funciton arguments supported in TSL.
enum class DataTypeEnum : int{
    INVALID = 0,
	VOID	,
	INT		,
	FLOAT	,
    DOUBLE  ,
	BOOL	,
    CLOSURE ,
	STRUCT
};

// A thin wrapper of data type with structure name if necessary.
struct DataType{
	DataTypeEnum	m_type;
	const char*		m_structure_name;	// this is only used for structure type
};

static inline bool operator == (DataType& data_type0, DataType& data_type1) {
	return data_type0.m_type == data_type1.m_type && data_type0.m_structure_name == data_type1.m_structure_name;
}

// Each argument could have some configuration
enum VariableConfig : int {
	NONE = 0,
	INPUT = 1,
	OUTPUT = 2,
	CONST = 4,
};

TSL_NAMESPACE_END