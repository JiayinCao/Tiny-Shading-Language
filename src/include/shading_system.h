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

#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "tslversion.h"

TSL_NAMESPACE_BEGIN

class ShadingContext;
class ShaderUnit;

//! @brief  Shading system is the root interface exposed through TSL system.
/*
 * A shading_system owns the whole TSL compiling system. A ray tracer with TSL integrated should have only one
 * instance of this class. Most of the interfaces, unless explicitly mentioned, should be thread-safe.
 * It owns all memory allocated of the system, it will also deallocate all memory allocated so there is no need
 * manually maintain memory allocated through this interface.
 */
class ShadingSystem {
public:
    //! @brief  Constructor.
    ShadingSystem();

    //! @brief  Destructor.
    ~ShadingSystem();

    //! @brief  Create a new shading context.
    //!
    //! Caller code doesn't need to release the memory allocated, it will be released automatically as the lifetime
    //! of this class comes to end.
    //!
    //! @return     Allocated memory points to an instance of a newly created shading_context.
    ShadingContext* make_shading_context();

private:
    std::unordered_set<std::unique_ptr<ShadingContext>>    m_contexts;         /**< Data structure holding all contexts. */
    std::mutex                                             m_context_mutex;    /**< Making sure context related operation is thread-safe. */

    /**< a mutex to make sure shader_group access is thread-safe. */
    std::mutex m_shader_unit_mutex;

    /**< a container holding all shader unit. */
    std::unordered_map<std::string, std::unique_ptr<ShaderUnit>> m_shader_units;

    friend class ShadingContext;
};

TSL_NAMESPACE_END