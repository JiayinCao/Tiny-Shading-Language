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

TSL_NAMESPACE_END