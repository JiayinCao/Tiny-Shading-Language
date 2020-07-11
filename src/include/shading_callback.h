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

#include "tslversion.h"
#include "export.h"

TSL_NAMESPACE_BEGIN

//! @brief  Debug information levels.
enum class TSL_DEBUG_LEVEL : unsigned int {
    TSL_DEBUG_INFO,         // General debugging information.
    TSL_DEBUG_WARNING,      // A warning means there is some badly written code in shader sources.
    TSL_DEBUG_ERROR,        // An error will most likely result failure in shader compilation.
};

//! @brief  ShadingSystem callback interface.
/**
 * ShadingSystemInterface offers a chance for renderers to do things like, outputing errors or logs, allocating memory for bxdf.
 * All methods in this interface need to be implemented in a thread-safe manner, it is renderer's job to make sure of it.
 * TSL won't synchronize upon calling these calls.
 */
class TSL_INTERFACE ShadingSystemInterface {
public:
    //! @brief  Virtual destructor.
    virtual ~ShadingSystemInterface() = default;

    //! @brief  Allocate memory inside shaders.
    //!
    //! There are things to be noticed in this interface.
    //!  - Shaders are not responsible to release the memory allocator allocates, it is up to the renderer to do so.
    //!  - This implementation has to be thread safe.
    virtual void*   allocate(unsigned int size) const = 0;

    //! @brief  This will be automatically called when there is error during shader compilation.
    //!
    //! @param  level       Debug level.
    //! @param  error       String describing the error.
    virtual void    catch_debug(const TSL_DEBUG_LEVEL level, const char* error) const = 0;

    //! @brief  Sample a 2d texture.
    //!
    //! @param  texture     Texture handle.
    //! @param  u           UV coordinate.
    //! @param  v           UV coordinate.
    //! @param  color       RGB of the texture pixel.
    virtual void    sample_2d(const void* texture, float u, float v, float3& color) const = 0;

    //! @brief  Sample alpha channel in a 2d texture.
    //!
    //! Having two separate interfaces for 2d sampling for RGB and alpha may not sound wise from a performance perspective.
    //! However, in order to get the ball rolling as soon as possible, I'll live with it now.
    //! To support float4 is also an alternative to be considered in the future.
    //!
    //! @param  texture     Texture handle.
    //! @param  u           UV coordinate.
    //! @param  v           UV coordinate.
    //! @param  alpha       Alpha channel of the texture.
    virtual void    sample_alpha_2d(const void* texture, float u, float v, float& alpha) const = 0;
};

TSL_NAMESPACE_END