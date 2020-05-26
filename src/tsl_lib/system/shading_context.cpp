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

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "shading_context.h"
#include "shading_system.h"
#include "compiler/compiler.h"
#include "compiler/shader_unit_pvt.h"
#include "compiler/global_module.h"

// a temporary ugly solution for debugging for now
// #define DEBUG_OUTPUT

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

bool ShaderUnit::resolve(){
    if(!m_shader_unit_data->m_module || !m_shader_unit_data->m_llvm_function)
        return false;

    auto module = m_shader_unit_data->m_module.get();
    
    // get the function pointer through execution engine
    m_shader_unit_data->m_execution_engine = std::unique_ptr<llvm::ExecutionEngine>(llvm::EngineBuilder(std::move(m_shader_unit_data->m_module)).create());

    auto execution_engine = m_shader_unit_data->m_execution_engine.get();

    // setup data layout
    module->setDataLayout(execution_engine->getTargetMachine()->createDataLayout());

#ifdef DEBUG_OUTPUT
	module->print(llvm::errs(), nullptr);
#endif

    // make sure to link the global closure model
    auto cloned_module = CloneModule(*(m_shader_unit_data->m_global_module->get_closure_module()));
    execution_engine->addModule(std::move(cloned_module));

    // resolve the function pointer
    const auto& function_name = m_shader_unit_data->m_root_function_name;
    m_shader_unit_data->m_function_pointer = execution_engine->getFunctionAddress(function_name);

    // optimization pass, this is pretty cool because I don't have to implement those sophisticated optimization algorithms.
    if(m_allow_optimization){
        // Create a new pass manager attached to it.
        m_shader_unit_data->m_fpm = std::make_unique<llvm::legacy::FunctionPassManager>(module);

        // Do simple "peephole" optimizations and bit-twiddling optzns.
        m_shader_unit_data->m_fpm->add(llvm::createInstructionCombiningPass());
        // Re-associate expressions.
        m_shader_unit_data->m_fpm->add(llvm::createReassociatePass());
        // Eliminate Common SubExpressions.
        m_shader_unit_data->m_fpm->add(llvm::createGVNPass());
        // Simplify the control flow graph (deleting unreachable blocks, etc).
        m_shader_unit_data->m_fpm->add(llvm::createCFGSimplificationPass());

        m_shader_unit_data->m_fpm->doInitialization();

        m_shader_unit_data->m_fpm->run(*m_shader_unit_data->m_llvm_function);
    }

    // make sure the function is valid
    if( m_allow_verification && !llvm::verifyFunction(*m_shader_unit_data->m_llvm_function, &llvm::errs()) )
    	return false;
    
    return true;
}

ShaderGroup::ShaderGroup(const std::string& name, const TslCompiler& compiler)
    :m_compiler(compiler), ShaderUnit(name){
}

bool ShaderGroup::resolve(){
    // if no root shader setup yet, return false
    if (m_root_shader_unit_name == "")
        return false;

    // if we can't find the root shader, it should return false
    if (0 == m_shader_units.count(m_root_shader_unit_name))
        return false;

    // essentially, this is a topological sort
    std::unordered_set<const ShaderUnit*>   m_visited_shader_units;

    // get the root shader
    auto root_shader = m_shader_units[m_root_shader_unit_name];

    // generate wrapper shader source code.
    generate_shader_source(root_shader, m_visited_shader_units);

    // to be implemented
    return true;
}

bool ShaderGroup::generate_shader_source(const ShaderUnit* su, std::unordered_set<const ShaderUnit*>& visited) {

    return false;
}

void ShaderGroup::connect_shader_units(const std::string& ssu, const std::string& sspn, const std::string& tsu, const std::string& tspn) {
    m_shader_unit_connections[ssu][sspn] = std::make_pair(tsu, tspn);
}

bool ShaderGroup::add_shader_unit(const ShaderUnit* shader_unit, const bool is_root) {
    if (!shader_unit)
        return false;

    // get the name of the shader
    const auto name = shader_unit->get_name();

    // if an existed shader is added
    if (m_shader_units.count(name)) {
        if (m_shader_units[name] != shader_unit)
            return false;
    }

    m_shader_units[shader_unit->get_name()] = shader_unit;

    if (is_root) {
        // more than one root shader set in the group
        if (m_root_shader_unit_name != "")
            return false;
        m_root_shader_unit_name = name;
    }

    return true;
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