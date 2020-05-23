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

enum DataTypeEnum : int{
	VOID	= 0,
	INT		,
	FLOAT	,
    DOUBLE  ,
	BOOL	,
	FLOAT3	,
	FLOAT4	,
	MATRIX	,
    CLOSURE ,
	STRUCT
};

struct DataType{
	DataTypeEnum	m_type;
	const char*		m_structure_name;	// this is only used for structure type
};

/*
static inline DataType operator = (DataType& data_type , DataTypeEnum type_enum) {
	data_type.m_type = type_enum;
	data_type.m_structure_name = "";
	return data_type;
}
*/

static inline bool operator == (DataType& data_type0, DataType& data_type1) {
	return data_type0.m_type == data_type1.m_type && data_type0.m_structure_name == data_type1.m_structure_name;
}

inline std::string str_from_data_type( const DataType& type ){
	switch( type.m_type ){
	case DataTypeEnum::INT:
		return "int";
	case DataTypeEnum::FLOAT:
		return "float";
	case DataTypeEnum::FLOAT3:
		return "float3";
	case DataTypeEnum::BOOL:
		return "bool";
	case DataTypeEnum::FLOAT4:
		return "float4";
	case DataTypeEnum::MATRIX:
		return "matrix";
    case DataTypeEnum::DOUBLE:
        return "double";
    case DataTypeEnum::CLOSURE:
        return "closure";
	case DataTypeEnum::STRUCT:
		return "struct " + std::string(type.m_structure_name);
	default:
		break;
	}

	assert( type.m_type == DataTypeEnum::VOID );
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