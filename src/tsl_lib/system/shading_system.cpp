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

#include <stdio.h>
#include <stdarg.h>
#include "shading_system.h"
#include "shading_context.h"
#include "compiler/global_module.h"
#include "llvm/Support/TargetSelect.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

/** This data structure is only accessable from shading system */
static std::unique_ptr<ShadingSystem_Impl> g_shading_system_impl = nullptr;

ShadingSystem& ShadingSystem::get_instance() {
    static ShadingSystem ss;
    return ss;
}

ShadingSystem::ShadingSystem() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    g_shading_system_impl = std::make_unique<ShadingSystem_Impl>();

    g_shading_system_impl->m_global_module = new GlobalModule();
    g_shading_system_impl->m_global_module->init();
}

ShadingSystem::~ShadingSystem() {
    g_shading_system_impl = nullptr;
}

ShadingContext* ShadingSystem::make_shading_context() {
    std::lock_guard<std::mutex> lock(g_shading_system_impl->m_context_mutex);

    auto shading_context = new ShadingContext(g_shading_system_impl.get());
    g_shading_system_impl->m_contexts.insert(std::unique_ptr<ShadingContext>(shading_context));
    return shading_context;
}

ClosureID ShadingSystem::register_closure_type(const std::string& name, ClosureVarList& mapping, int structure_size) {
    return g_shading_system_impl->m_global_module->register_closure_type(name, mapping, structure_size);
}

void ShadingSystem::register_tsl_global(GlobalVarList& mapping) {
    return g_shading_system_impl->m_global_module->register_tsl_global(mapping);
}

void ShadingSystem::register_shadingsystem_interface(std::unique_ptr<ShadingSystemInterface> ssi){
    g_shading_system_impl->m_callback = std::move(ssi);
}

void* allocate_memory(const unsigned size) {
    const auto callback = g_shading_system_impl->m_callback.get();
    return callback ? callback->allocate(size) : nullptr;
}

void  emit_error(const char* format, ...) {
    va_list argList;
    
    // 1MB should be good enough for most errors.
    constexpr int max_buf_size = 1024 * 1024;

    std::unique_ptr<char[]> buf = std::make_unique<char[]>(max_buf_size);
    va_start(argList, format);
    vsprintf(buf.get(), format, argList);
    va_end(argList);

    const auto callback = g_shading_system_impl->m_callback.get();
    if (callback)
        callback->catch_debug(ShadingSystemInterface::DEBUG_ERROR, buf.get());
}

TSL_NAMESPACE_END
