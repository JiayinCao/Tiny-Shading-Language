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

#include <memory>
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "shading_context.h"
#include "shading_system.h"
#include "compiler/compiler.h"
#include "compiler/shader_unit_pvt.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

ShaderUnitTemplate::ShaderUnitTemplate(const std::string& name){
    m_shader_unit_template_impl = new ShaderUnitTemplate_Impl();
    m_shader_unit_template_impl->m_shader_unit_data = new ShaderUnitTemplate_Pvt();
    m_shader_unit_template_impl->m_name = name;
}

ShaderUnitTemplate::~ShaderUnitTemplate(){
    delete m_shader_unit_template_impl->m_shader_unit_data;
    delete m_shader_unit_template_impl;
}

const std::string& ShaderUnitTemplate::get_name() const {
    return m_shader_unit_template_impl->m_name;
}

std::unique_ptr<ShaderInstance> ShaderUnitTemplate::make_shader_instance() {
    return std::make_unique<ShaderInstance>(*this);
}

ShaderInstance::ShaderInstance(const ShaderUnitTemplate& sut) : m_shader_unit_template(sut) {
    m_shader_instance_data = new ShaderInstance_Pvt();
}

ShaderInstance::~ShaderInstance() {
    delete m_shader_instance_data;
}

uint64_t ShaderInstance::get_function() const {
    return m_shader_instance_data->m_function_pointer;
}

void ShaderUnitTemplate::parse_dependencies(ShaderUnitTemplate_Pvt* sut) const {
    if (!sut)
        return;
    sut->m_dependencies.insert(m_shader_unit_template_impl->m_shader_unit_data->m_module.get());
}

bool ShaderUnitTemplate::register_texture(const std::string name, const TextureHandle* th) {
    if (!m_shader_unit_template_impl->m_shader_texture_table.count(name))
        return false;
    if (!th)
        return false;
    m_shader_unit_template_impl->m_shader_texture_table[name]->m_texture_handle = th;
    return true;
}

ShaderGroupTemplate::ShaderGroupTemplate(const std::string& name)
    :ShaderUnitTemplate(name){
    m_shader_group_template_impl = new ShaderGroupTemplate_Impl();
}

ShaderGroupTemplate::~ShaderGroupTemplate() {
    delete m_shader_group_template_impl;
}

void ShaderGroupTemplate::connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn) {
    m_shader_group_template_impl->m_shader_unit_connections[tsu][tspn] = std::make_pair(ssu, sspn);
}

void ShaderGroupTemplate::expose_shader_argument(const std::string & ssu, const std::string & sspn, const ArgDescriptor & arg_desc){
    const auto i = m_shader_unit_template_impl->m_exposed_args.size();
    m_shader_unit_template_impl->m_exposed_args.push_back(arg_desc);

    // I may need to do some checking here to make sure things don't get setup in an invalid way
    if (arg_desc.m_is_output)
        m_shader_group_template_impl->m_output_args[ssu][sspn] = i;
    else
        m_shader_group_template_impl->m_input_args[ssu][sspn] = i;
}


void ShaderGroupTemplate::init_shader_input(const std::string& su, const std::string& spn, const ShaderUnitInputDefaultValue& val) {
    m_shader_group_template_impl->m_shader_input_defaults[su][spn] = val;
}

void ShaderGroupTemplate::parse_dependencies(ShaderUnitTemplate_Pvt* sut) const {
    for (const auto& shader_unit : m_shader_group_template_impl->m_shader_units)
        shader_unit.second.m_shader_unit_template->parse_dependencies(sut);
    sut->m_dependencies.insert(m_shader_unit_template_impl->m_shader_unit_data->m_module.get());
}

bool ShaderGroupTemplate::add_shader_unit(const std::string& name, ShaderUnitTemplate* shader_unit, const bool is_root) {
    if (!shader_unit)
        return false;

    // if an existed shader is added
    if (m_shader_group_template_impl->m_shader_units.count(name))
        return false;

    m_shader_group_template_impl->m_shader_units[name].m_name = name;
    m_shader_group_template_impl->m_shader_units[name].m_shader_unit_template = shader_unit;

    if (is_root) {
        // more than one root shader set in the group
        if (m_shader_group_template_impl->m_root_shader_unit_name != "")
            return false;
        m_shader_group_template_impl->m_root_shader_unit_name = name;
    }

    return true;
}

ShadingContext::ShadingContext(ShadingSystem_Impl* shading_system_impl){
    m_shading_context_impl = new ShadingContext_Impl();
    m_shading_context_impl->m_shading_system_impl = shading_system_impl;
    m_shading_context_impl->m_compiler = std::make_unique<TslCompiler>(*shading_system_impl->m_global_module);
}

ShadingContext::~ShadingContext() {
    delete m_shading_context_impl;
}

ShaderUnitTemplate* ShadingContext::compile_shader_unit_template(const std::string& name, const char* source) const {
    // make sure the lock doesn't cover compiling
    {
        // making sure only one of the context can access the data at a time
        std::lock_guard<std::mutex> lock(m_shading_context_impl->m_shading_system_impl->m_shader_unit_mutex);

        // if the shader group is created before, return nullptr.
        if (m_shading_context_impl->m_shading_system_impl->m_shader_units.count(name))
            return nullptr;

        // allocate the shader unit entry
        m_shading_context_impl->m_shading_system_impl->m_shader_units[name] = std::make_unique<ShaderUnitTemplate>(name);
    }

    auto shader_unit = m_shading_context_impl->m_shading_system_impl->m_shader_units[name].get();

    // compile the shader unit
    const bool ret = m_shading_context_impl->m_compiler->compile(source, shader_unit);
    if (!ret)
        return nullptr;

    return shader_unit;
}

TSL_Resolving_Status ShadingContext::end_shader_group_template(ShaderGroupTemplate* sg) const {
    return m_shading_context_impl->m_compiler->resolve(sg);
}

TSL_Resolving_Status ShadingContext::resolve_shader_instance(ShaderInstance* si) const {
    return m_shading_context_impl->m_compiler->resolve(si);
}

ShaderGroupTemplate* ShadingContext::begin_shader_group_template(const std::string& name) {
    // making sure only one of the context can access the data at a time
    std::lock_guard<std::mutex> lock(m_shading_context_impl->m_shading_system_impl->m_shader_unit_mutex);

    // if the shader group is created before, return nullptr.
    if (m_shading_context_impl->m_shading_system_impl->m_shader_units.count(name))
        return nullptr;

    auto shader_group = new ShaderGroupTemplate(name);
    m_shading_context_impl->m_shading_system_impl->m_shader_units[name] = std::unique_ptr<ShaderGroupTemplate>(shader_group);
    return shader_group;
}

TSL_NAMESPACE_END
