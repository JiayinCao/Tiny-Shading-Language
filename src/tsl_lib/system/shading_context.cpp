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

#include "shading_context.h"
#include "shading_system.h"
#include "compiler/compiler.h"
#include "compiler/shader_unit_pvt.h"
#include "compiler/global_module.h"

TSL_NAMESPACE_BEGIN

ShaderUnit::ShaderUnit(const std::string& name) :m_name(name) {
    m_shader_unit_data = new ShaderUnit_Pvt();
}

ShaderUnit::~ShaderUnit(){
    delete m_shader_unit_data;
}

uint64_t ShaderUnit::get_function() const{
    return m_shader_unit_data->m_function_pointer;
}

ShaderGroup::ShaderGroup(const std::string& name, const TslCompiler& compiler)
    :m_compiler(compiler), ShaderUnit(name){
}

bool ShaderGroup::compile() {
    // to be implemented
    return false;
}

void ShaderGroup::add_shader_group(const ShaderUnit* shader_unit) {
    if (!shader_unit)
        return;

    m_shader_units[shader_unit->get_name()] = shader_unit;
}

ShadingContext::ShadingContext(ShadingSystem& shading_system):m_shading_system(shading_system) {
    m_compiler = std::make_unique<TslCompiler>(*m_shading_system.m_global_module);
}

ShadingContext::~ShadingContext() {
}

ShaderUnit* ShadingContext::compile_shader_unit(const std::string& name, const char* source) const {
    // make sure the lock doesn't cover compiling
    {
        // making sure only one of the context can access the data at a time
        std::lock_guard<std::mutex> lock(m_shading_system.m_shader_unit_mutex);

        // if the shader group is created before, return nullptr.
        if (m_shading_system.m_shader_units.count(name))
            return nullptr;

        // allocate the shader unit entry
        m_shading_system.m_shader_units[name] = std::make_unique<ShaderUnit>(name);
    }

    auto shader_unit = m_shading_system.m_shader_units[name].get();

    // compile the shader unit
    const bool ret = m_compiler->compile(source, shader_unit);
    if (!ret)
        return nullptr;

    return shader_unit;
}

ShaderGroup* ShadingContext::make_shader_group(const std::string& name) {
    // making sure only one of the context can access the data at a time
    std::lock_guard<std::mutex> lock(m_shading_system.m_shader_unit_mutex);

    // if the shader group is created before, return nullptr.
    if (m_shading_system.m_shader_units.count(name))
        return nullptr;

    auto shader_group = new ShaderGroup(name, *m_compiler);
    m_shading_system.m_shader_units[name] = std::unique_ptr<ShaderUnit>(shader_group);
    return shader_group;
}

TSL_NAMESPACE_END