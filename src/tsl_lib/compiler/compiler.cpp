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

#include "compiler.h"
#include "compiler_impl.h"

TSL_NAMESPACE_BEGIN

TslCompiler::TslCompiler(GlobalModule& global_module){
    m_compiler_impl = std::make_unique<TslCompiler_Impl>(global_module);
}

bool TslCompiler::compile(const char* source_code, ShaderUnitTemplate* su) const {
    return m_compiler_impl->compile(source_code, su);
}

bool TslCompiler::resolve(ShaderGroupTemplate* sg) const {
    return m_compiler_impl->resolve(sg);
}

bool TslCompiler::resolve(ShaderInstance* si) const {
    return m_compiler_impl->resolve(si);
}

TSL_NAMESPACE_END