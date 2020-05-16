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

#include <assert.h>
#include "tslversion.h"

TSL_NAMESPACE_BEGIN

enum DataType : int{
	VOID	= 0,
	INT		,
	FLOAT	,
	BOOL	,
	FLOAT3	,
	FLOAT4	,
	MATRIX	
};

inline const char* str_from_data_type( const DataType type ){
	switch( type ){
	case DataType::INT:
		return "int";
	case DataType::FLOAT:
		return "float";
	case DataType::FLOAT3:
		return "float3";
	case DataType::BOOL:
		return "bool";
	case DataType::FLOAT4:
		return "float4";
	case DataType::MATRIX:
		return "matrix";
	default:
		break;
	}

	assert( type == DataType::VOID );
	return "void";
}

enum VariableConfig : int {
	NONE = 0,
	INPUT = 1,
	OUTPUT = 2,
	CONST = 4,
};

inline const char* str_from_var_config(const VariableConfig type) {
	if( type == ( VariableConfig::NONE ) )
		return "";

	if( type == ( VariableConfig::CONST ) )
		return "const";

	if( type == (VariableConfig::CONST | VariableConfig::INPUT) )
		return "const in";

	// this is useless
	//if (type == (VariableConfig::CONST | VariableConfig::OUTPUT))
	//	return "const output";

	if( type == VariableConfig::INPUT )
		return "in";

	if( type == VariableConfig::OUTPUT )
		return "out";

	return "invalid";
}

TSL_NAMESPACE_END