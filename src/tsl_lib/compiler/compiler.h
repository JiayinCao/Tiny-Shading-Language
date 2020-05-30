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
#include <string>
#include "tslversion.h"
#include "compiler_impl.h"

TSL_NAMESPACE_BEGIN

class TslCompiler_Impl;

//! @Brief  This is just a wrapper to make sure things are clean.
/*
 * TslCompiler is responsible for compiling source code so that it can be output as serialized string.
 * This is just a thin warpper for hiding the detailed implementaiton of the compiler itself.
 */
class TslCompiler {
public:
    //! Constructor does nothing but to construct the m_compiler data.
    TslCompiler(GlobalModule& global_module);

    //! @brief  Compile a shader.
    //!
    //! Ideally, this should be thread-safe. Flex and Bison support it, as long as I can make sure LLVM supports too,
    //! it should be thread-safe.
    //!
    //! @param  source_code     The source code of the shader module.
    //! @param  su              The shader unit owning this piece of source code.
    bool compile(const char* source_code, ShaderUnitTemplate* su) const;
    
    //! @brief  Resolve a shader unit
    //!
    //! All shader unit needs to be resolved after compiling.
    //! @param  su              The shader unit to be resolved.
    //! @return                 Whether the shader unit is resolved succesfully.
    bool resolve(ShaderUnitTemplate* su) const;

    //! @brief  Resolve a shader instance.
    //!
    //! A shader instance needs to be resolved before being put in use.
    //! @return                 Whether the shader instance is successfully resolved.
    bool resolve(ShaderInstance* si) const;

private:
    std::unique_ptr<TslCompiler_Impl> m_compiler_impl = nullptr;
};

TSL_NAMESPACE_END