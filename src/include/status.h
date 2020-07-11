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

enum TSL_Resolving_Status {
    TSL_Resolving_Succeed = 0,
    TSL_Resolving_InvalidInput,                     /*< Input is nullptr. */
    TSL_Resolving_ShaderGroupWithoutRoot,           /*< No shader unit defined as root in the input shader group. */
    TSL_Resolving_ShaderGroupWithCycles,            /*< There is cycles detected in the shader group. */
    TSL_Resolving_InvalidShaderGroupTemplate,       /*< Invalid shader group template. */
    TSL_Resolving_LLVMFunctionVerificationFailed,   /*< LLVM verfication of the */
    TSL_Resolving_UndefinedShaderUnit,              /*< A specific shader unit is not defined in the shader group. */
    TSL_Resolving_InvalidArgType,                   /*< One of the input arguments defined in the shader group template is not defined. */
    TSL_Resolving_ArgumentWithoutInitialization,    /*< One of the arguments passed in doesn't have a valid initialization value. */
    TSL_Resolving_UnspecifiedError                  /*< This error type is not specified somehow. */
};

TSL_NAMESPACE_END
