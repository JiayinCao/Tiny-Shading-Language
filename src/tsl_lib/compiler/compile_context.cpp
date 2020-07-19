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

#include "compile_context.h"

TSL_NAMESPACE_BEGIN

void TSL_Compile_Context::reset() {
    m_var_symbols.clear();
    m_var_symbols.push_back({});    // this is for global variables
}

llvm::Value* TSL_Compile_Context::get_var_symbol(const std::string& name, bool only_top_layer) {
    if (only_top_layer) {
        auto top = m_var_symbols.back();
        auto it = top.find(name);
        return it == top.end() ? nullptr : it->second.first;
    }
    else {
        auto it = m_var_symbols.rbegin();
        while (it != m_var_symbols.rend()) {
            auto var = it->find(name);
            if (var != it->end())
                return var->second.first;
            ++it;
        }
    }

    emit_error("Undefined variable '%s'.", name.c_str());

    return nullptr;
}

DataType TSL_Compile_Context::get_var_type(const std::string& name, bool only_top_layer) {
    if (only_top_layer) {
        auto top = m_var_symbols.back();
        auto it = top.find(name);
        if (it != top.end())
            return it->second.second;
    }
    else {
        auto it = m_var_symbols.rbegin();
        while (it != m_var_symbols.rend()) {
            auto var = it->find(name);
            if (var != it->end())
                return var->second.second;
            ++it;
        }
    }

    emit_error("Undefined variable '%s'.", name.c_str());

    return DataType();
}

llvm::Value* TSL_Compile_Context::push_var_symbol(const std::string& name, llvm::Value* value, DataType type) {
    auto top_layer = m_var_symbols.back();

    if (top_layer.count(name)) {
        emit_error("Redefined variable '%s'.", name.c_str());
        return nullptr;
    }

    m_var_symbols.back()[name] = std::make_pair(value, type);

    return nullptr;
}

void TSL_Compile_Context::push_var_symbol_layer() {
    m_var_symbols.push_back({});
}

void TSL_Compile_Context::pop_var_symbol_layer() {
    m_var_symbols.pop_back();
}

TSL_NAMESPACE_END
