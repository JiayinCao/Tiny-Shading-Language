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

#include "shading_system.h"
#include "shading_context.h"
#include "compiler/global_module.h"
#include "llvm/Support/TargetSelect.h"

TSL_NAMESPACE_BEGIN

ShadingSystem::ShadingSystem() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    m_global_module = new GlobalModule();
    m_global_module->init();
}

ShadingSystem::~ShadingSystem() {
//    if (m_global_module)
//        delete m_global_module;
}

ShadingContext* ShadingSystem::make_shading_context() {
    std::lock_guard<std::mutex> lock(m_context_mutex);

    auto shading_context = new ShadingContext(*this);
    auto iter = m_contexts.insert(std::unique_ptr<ShadingContext>(shading_context));
    return shading_context;
}

ClosureID ShadingSystem::register_closure_type(const std::string& name, ClosureVarList& mapping, int structure_size) {
    return m_global_module->register_closure_type(name, mapping, structure_size);
}

template<class T>
void test(){
	std::cout<<"am"<<std::endl;
}

TSL_NAMESPACE_END