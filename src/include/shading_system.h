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
#include "shading_callback.h"
#include "common.h"

TSL_NAMESPACE_BEGIN

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

    //! @brief  Make the only instance of shading system in TSL system.
    //!
    //! This has to be called before anything in TSL called. The interface to be registered is very important to shader
    //! compilation. ShadingSystem will take over the ownership of the pointer passed in. Renderers don't need to 
    //! deallocate the memory of the passed in parameter, it will also need to avoid access of this parameter in renderer 
    //! later.
    //!
    //! @param  ssi         The interface to be registered.
    static void             register_shadingsystem_interface(std::unique_ptr<ShadingSystemInterface> ssi);

    //! @brief  Get shading system instance.
    //!
    //! In order to make sure there is not a second instance of shading system in renderers, this class is a class of singleton.
    //! There is no way to have a second instance of it, which is secured duing compilation time.
    //!
    //! @return             Reference to the only instance of the class.
    static ShadingSystem&   get_instance();

    //! @brief  Create a new shading context.
    //!
    //! TSL shading system won't take responsibility of keeping shading context alive. It is up to renderers to make sure it is alive
    //! when it is still needed. However, shading context life time will also be observed by things like shader unit template and
    //! shader instance, meaning as long as there is a shader instance or shader unit template alive, the context which creates them
    //! will also be alive.
    //!
    //! @return             A smart pointer to the newly created shading context returned by TSL system.
    std::shared_ptr<class ShadingContext>  make_shading_context();

    //! @brief  Register closure id.
    //!
    //! @param  name            Name of the closure. This has to match the one used in TSL shaders.
    //! @param  mapping         Mapping of the data inside the closure.
    //! @param  closure_size    Size of the data structure.
    //! @return                 Allocated closure id for the closure.
    ClosureID               register_closure_type(const std::string& name, ClosureVarList& mapping, int closure_size);

    //! @brief  Register tsl global data.
    //!
    //! @param  mapping     Mapping of the data structure.
    void                    register_tsl_global(GlobalVarList& mapping);
    
    TSL_HIDE_CONSTRUCTOR(ShadingSystem)
};

TSL_NAMESPACE_END