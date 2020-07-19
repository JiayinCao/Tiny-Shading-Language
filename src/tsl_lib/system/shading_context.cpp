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
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include "tsl_system.h"
#include "compiler/compiler.h"
#include "system/impl.h"
#include "tsl_args.h"

TSL_NAMESPACE_BEGIN

namespace {
    // Cyclic redundancy check
    // https://en.wikipedia.org/wiki/Cyclic_redundancy_check#CRC-32_algorithm
    //
    // Computation of cyclic redundancy checks
    // https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
    //
    // Fast CRC32
    // https://create.stephan-brumme.com/crc32/#bitwise

    static constexpr unsigned int Polynomial = 0xEDB88320;

    static unsigned int crc32_bitwise(const std::string& s) {
        const char* data = s.c_str();
        unsigned length = s.length();
        unsigned int crc = 0;
        while (length--) {
            crc ^= *data++;

            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        }
        return ~crc;
    }

    const unsigned generated_hash(const std::vector<GlobalVar>& var_list) {
        unsigned hash = 0;
        for (const auto& global_var : var_list) {
            hash = hash ^ crc32_bitwise(global_var.m_name);
            hash = hash ^ crc32_bitwise(global_var.m_type);
        }
        return 0;
    }
}

ShaderUnitTemplate::ShaderUnitTemplate(const std::string& name, std::shared_ptr<ShadingContext> context){
    // this means that it is a shader group template.
    if (context == nullptr)
        return;

    m_shader_unit_template_impl = std::make_shared<ShaderUnitTemplate_Impl>();
    m_shader_unit_template_impl->m_name = name;
    m_shader_unit_template_impl->m_shading_context = context;
}

const std::string& ShaderUnitTemplate::get_name() const {
    return m_shader_unit_template_impl->m_name;
}

std::shared_ptr<ShaderInstance> ShaderUnitTemplate::make_shader_instance() {
    // Ideally, I should have used make_shared instead of shared_ptr to construct this shader instance.
    // However, in order to hide the useless interface from user of TSL, I chose to hide the constructor,
    // leaving TSL users no choice to construct shader instance but to go through shader unit template.
    return std::shared_ptr<ShaderInstance>(new ShaderInstance(shared_from_this()));
}

bool ShaderUnitTemplate::register_tsl_global(const GlobalVarList& tslg) {
    if (!m_shader_unit_template_impl->m_tsl_global.m_var_list.empty()) {
        emit_warning("TSL global already registered in shader unit template %s.", get_name().c_str());
        return false;
    }
    m_shader_unit_template_impl->m_tsl_global = tslg;
    m_shader_unit_template_impl->m_tsl_global_hash = generated_hash(tslg.m_var_list);
    return true;
}

ShaderInstance::ShaderInstance(std::shared_ptr<ShaderUnitTemplate> sut) {
    m_shader_instance_data = std::make_shared<ShaderInstance_Impl>();
    m_shader_instance_data->m_shader_unit_template = sut;
}

uint64_t ShaderInstance::get_function() const {
    return m_shader_instance_data->m_function_pointer;
}

bool ShaderUnitTemplate::register_shader_resource(const std::string& name, const ShaderResourceHandle* srh) {
    if (m_shader_unit_template_impl->m_shader_resource_table.count(name))
        return false;
    if (!srh)
        return false;
    m_shader_unit_template_impl->m_shader_resource_table[name] = srh;
    return true;
}

ShaderGroupTemplate::ShaderGroupTemplate(const std::string& name, std::shared_ptr<ShadingContext> context)
    :ShaderUnitTemplate("", nullptr){
    m_shader_unit_template_impl = std::make_shared<ShaderGroupTemplate_Impl>();
    m_shader_unit_template_impl->m_name = name;
    m_shader_unit_template_impl->m_shading_context = context;
}

void ShaderGroupTemplate::connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn) {
    ShaderGroupTemplate_Impl* sg_impl = (ShaderGroupTemplate_Impl*) m_shader_unit_template_impl.get();
    sg_impl->m_shader_unit_connections[tsu][tspn] = std::make_pair(ssu, sspn);
}

void ShaderGroupTemplate::expose_shader_argument(const std::string& su, const std::string& spn, const bool is_output, const std::string& name){
    ExposedArgDescriptor arg_desc;
    arg_desc.m_is_output = is_output;
    arg_desc.m_name = name.empty() ? spn : name;
    arg_desc.m_source_shader_unit_name = su;
    arg_desc.m_source_shader_unit_arg_name = spn;

    const auto i = m_shader_unit_template_impl->m_exposed_args.size();
    m_shader_unit_template_impl->m_exposed_args.push_back(arg_desc);

    // I may need to do some checking here to make sure things don't get setup in an invalid way
    ShaderGroupTemplate_Impl* sg_impl = (ShaderGroupTemplate_Impl*)m_shader_unit_template_impl.get();
    if (arg_desc.m_is_output)
        sg_impl->m_output_args[su][spn] = i;
    else
        sg_impl->m_input_args[su][spn] = i;
}

void ShaderGroupTemplate::init_shader_input(const std::string& su, const std::string& spn, const ShaderUnitInputDefaultValue& val) {
    ShaderGroupTemplate_Impl* sg_impl = (ShaderGroupTemplate_Impl*)m_shader_unit_template_impl.get();
    sg_impl->m_shader_input_defaults[su][spn] = val;
}

bool ShaderGroupTemplate::add_shader_unit(const std::string& name, std::shared_ptr<ShaderUnitTemplate> shader_unit, const bool is_root) {
    if (!shader_unit)
        return false;

    ShaderGroupTemplate_Impl* sg_impl = (ShaderGroupTemplate_Impl*)m_shader_unit_template_impl.get();

    // if an existed shader is added
    if (sg_impl->m_shader_units.count(name))
        return false;

    sg_impl->m_shader_units[name].m_name = name;
    sg_impl->m_shader_units[name].m_shader_unit_template = shader_unit;

    if (is_root) {
        // more than one root shader set in the group
        if (sg_impl->m_root_shader_unit_name != "")
            return false;
        sg_impl->m_root_shader_unit_name = name;
    }

    return true;
}

ShadingContext::ShadingContext(std::shared_ptr<ShadingSystem_Impl> shading_system_impl){
    m_shading_context_impl = std::make_shared<ShadingContext_Impl>();
    m_shading_context_impl->m_shading_system_impl = shading_system_impl;
    m_shading_context_impl->m_compiler = std::make_unique<TslCompiler>(*shading_system_impl->m_global_module);
}

std::shared_ptr<ShaderUnitTemplate> ShadingContext::begin_shader_unit_template(const std::string& name) {
    auto ptr = shared_from_this();
    return std::shared_ptr<ShaderUnitTemplate>(new ShaderUnitTemplate(name, ptr));
}

TSL_Resolving_Status ShadingContext::end_shader_unit_template(ShaderUnitTemplate* su) const {
    // nothing needs to be done here
    return TSL_Resolving_Status::TSL_Resolving_Succeed;
}

bool  ShadingContext::compile_shader_unit_template(ShaderUnitTemplate * sut, const char* source) const {
    return m_shading_context_impl->m_compiler->compile(source, sut);
}

TSL_Resolving_Status ShadingContext::end_shader_group_template(ShaderGroupTemplate* sg) const {
    return m_shading_context_impl->m_compiler->resolve(sg);
}

TSL_Resolving_Status ShadingContext::resolve_shader_instance(ShaderInstance* si) const {
    return m_shading_context_impl->m_compiler->resolve(si);
}

std::shared_ptr<ShaderGroupTemplate> ShadingContext::begin_shader_group_template(const std::string& name) {
    return std::shared_ptr<ShaderGroupTemplate>(new ShaderGroupTemplate(name, shared_from_this()));
}

void ShaderGroupTemplate_Impl::parse_dependencies(ShaderUnitTemplate_Impl* sut) const {
    for (const auto& shader_unit : m_shader_units)
        shader_unit.second.m_shader_unit_template->m_shader_unit_template_impl->parse_dependencies(sut);
    sut->m_dependencies.insert(m_module.get());
}

void ShaderUnitTemplate_Impl::parse_dependencies(ShaderUnitTemplate_Impl* sut) const {
    if (!sut)
        return;
    sut->m_dependencies.insert(m_module.get());
}

GlobalVarList::GlobalVarList(const std::vector<GlobalVar>& var_list) :
    m_var_list(var_list) /* keep track of the variable list. */{
}

GlobalVarList::GlobalVarList(){
}

GlobalVarList::GlobalVarList(const GlobalVarList& copy) :
    m_var_list(copy.m_var_list){
}

TSL_NAMESPACE_END
