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
#include <llvm/Support/TargetSelect.h>
#include "tsl_system.h"
#include "compiler/global_module.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

/** This data structure is only accessable from shading system */
static std::shared_ptr<ShadingSystem_Impl> g_shading_system_impl = nullptr;

ShadingSystem& ShadingSystem::get_instance() {
    static ShadingSystem ss;
    return ss;
}

ShadingSystem::ShadingSystem() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    g_shading_system_impl = std::make_shared<ShadingSystem_Impl>();

    g_shading_system_impl->m_global_module = std::make_unique<GlobalModule>();
    g_shading_system_impl->m_global_module->init();
}

ShadingSystem::~ShadingSystem() {
    g_shading_system_impl = nullptr;
}

std::shared_ptr<ShadingContext> ShadingSystem::make_shading_context() {
    return std::shared_ptr<ShadingContext>(new ShadingContext(g_shading_system_impl));
}

ClosureID ShadingSystem::register_closure_type(const std::string& name, ClosureArgList& mapping, int structure_size) {
    return g_shading_system_impl->m_global_module->register_closure_type(name, mapping, structure_size);
}

void ShadingSystem::register_shadingsystem_interface(std::unique_ptr<ShadingSystemInterface> ssi){
    g_shading_system_impl->m_callback = std::move(ssi);
}

void* allocate_memory(const unsigned size, void* ptr) {
    const auto callback = g_shading_system_impl->m_callback.get();
    return callback ? callback->allocate(size, ptr) : nullptr;
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
        callback->catch_debug(TSL_DEBUG_LEVEL::TSL_DEBUG_ERROR, buf.get());
}

void  emit_warning(const char* format, ...) {
    va_list argList;

    // 1MB should be good enough for most errors.
    constexpr int max_buf_size = 1024 * 1024;

    std::unique_ptr<char[]> buf = std::make_unique<char[]>(max_buf_size);
    va_start(argList, format);
    vsprintf(buf.get(), format, argList);
    va_end(argList);

    const auto callback = g_shading_system_impl->m_callback.get();
    if (callback)
        callback->catch_debug(TSL_DEBUG_LEVEL::TSL_DEBUG_WARNING, buf.get());
}

void  sample_2d(const void* texture, float u, float v, float3& color) {
    const auto callback = g_shading_system_impl->m_callback.get();
    if (nullptr == callback)
        return;
    callback->sample_2d(texture, u, v, color);
}

void  sample_alpha_2d(const void* texture, float u, float v, float& alpha) {
    const auto callback = g_shading_system_impl->m_callback.get();
    if (nullptr == callback)
        return;
    callback->sample_alpha_2d(texture, u, v, alpha);
}

TSL_NAMESPACE_END
