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
#include "closure.h"
#include "global.h"
#include "export.h"
#include "shader_arg_types.h"

TSL_NAMESPACE_BEGIN

class ShadingContext;

//! @brief  ShadingSystem callback interface.
/**
 * ShadingSystemInterface offers a chance for renderers to do things like, output error or log, allocate memory for bxdf.
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

    //! @brief  Debug information levels.
    enum DEBUG_LEVEL {
        DEBUG_INFO,
        DEBUG_WARNING,
        DEBUG_ERROR,
    };

    //! @brief  This will be called when there is error during shader compilation.
    //!
    //! @param  level       Debug level.
    //! @param  error       String describing the error.
    virtual void    catch_debug(const DEBUG_LEVEL level, const char* error) const = 0;

    //! @brief  Sample 2d texture.
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

//! @brief  Shading system is the root interface exposed through TSL system.
/*
 * A shading_system owns the whole TSL compiling system. A ray tracer with TSL integrated should have only one
 * instance of this class. Most of the interfaces, unless explicitly mentioned, should be thread-safe.
 * It owns all memory allocated of the system, it will also deallocate all memory allocated so there is no need
 * manually maintain memory allocated through this interface.
 */
class TSL_INTERFACE ShadingSystem {
public:
    //! @brief  Destructor.
    ~ShadingSystem();

    //! @brief  Get shading system instance
    //!
    //! Shading system is a singleton.
    static ShadingSystem& get_instance();

    //! @brief  Create a new shading context.
    //!
    //! Caller code doesn't need to release the memory allocated, it will be released automatically as the lifetime
    //! of this class comes to end.
    //!
    //! @return             Allocated memory points to an instance of a newly created shading_context.
    ShadingContext*         make_shading_context();

    //! @brief  Register closure id.
    //!
    //! @param  name            Name of the closure.
    //! @param  mapping         Mapping of the data inside the closure.
    //! @param  closure_size    Size of the data structure.
    //! @return                 Allocated closure id for the closure.
    ClosureID               register_closure_type(const std::string& name, ClosureVarList& mapping, int closure_size);

    //! @brief  Register tsl global data.
    //!
    //! @param  mapping     Mapping of the data structure.
    void                    register_tsl_global(GlobalVarList& mapping);

    //! @brief  Register ShadingSystemInterface.
    //!
    //! ShadingSystem will take over the ownership of the pointer passed in. Renderers don't need to deallocate the memory
    //! of the passed in parameter, it will also need to avoid access of this parameter in renderer later.
    //!
    //! @param  ssi     The interface to be registered.
    void                    register_shadingsystem_interface(std::unique_ptr<ShadingSystemInterface> ssi);

private:
    //! @brief  Constructor.
    //!
    //! Making sure the shading system only has one single instance.
    ShadingSystem();

    //! @brief  Copy Constructor.
    //!
    //! Making sure the shading system only has one single instance.
    ShadingSystem(const ShadingSystem&) { /* simply don't allow it. */ }
};

TSL_NAMESPACE_END