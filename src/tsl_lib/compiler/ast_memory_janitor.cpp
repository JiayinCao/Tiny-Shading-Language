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
#include <unordered_map>
#include "ast_memory_janitor.h"
#include "compiler/ast.h"

TSL_NAMESPACE_BEGIN

//! @brief  This class will make sure all TSL allocated memory will be registered in it and eventually it will be destroyed.
class TSL_Memory_Janior {
public:
    //! @brief  Keep track of this node
    void track_ast_node(const AstNode* node) {
        auto ptr = std::shared_ptr<const AstNode>(node);
        m_ast_nodes[node] = ptr;
    }

    //! @brief  Find ast node shared_ptr
    std::shared_ptr<const AstNode> find_shared_ptr(const AstNode* node) {
        return m_ast_nodes.count(node) ? m_ast_nodes[node] : nullptr;
    }

private:
    // This data structure keeps track of all ast nodes' life time.
    // However, since some of the Ast Node is also owned by other data structures, there is a chance that even if this janitor
    // is deallocated and the Ast Nodes are still alive. But since every other part of the problem uses a shared_ptr, it should
    // be safe to say there won't be any memory leak caused by AstNode.
    std::unordered_map<const AstNode*, std::shared_ptr<const AstNode>>  m_ast_nodes;
};

// This container is purely for the purpose of keeping track of the life time of AstNodes.
// As a matter of fact, it is more for incorrect shaders where the compiler fails to compile during parsing because some of the
// ast node will get dangled leaving memory leak. For correct shaders, all nodes will be owned by something in the shader template,
// which eventually will be destroyed at some point.
// In order to make it thread safe, thread local storage is needed here to prevent data racing among threads.
thread_local static std::vector<TSL_Memory_Janior>   g_tsl_memory_janitor_stack;

Ast_Memory_Guard::Ast_Memory_Guard() {
    g_tsl_memory_janitor_stack.push_back(TSL_Memory_Janior());
}

Ast_Memory_Guard::~Ast_Memory_Guard() {
    g_tsl_memory_janitor_stack.pop_back();
}

void ast_ptr_tracking(const AstNode* node) {
    assert(nullptr == ast_ptr_from_raw<AstNode>(node));

    if (g_tsl_memory_janitor_stack.size()) {
        auto& janitor = g_tsl_memory_janitor_stack.back();
        janitor.track_ast_node(node);
    }
}

template<class T>
std::shared_ptr<const T>  ast_ptr_from_raw(const AstNode* ptr) {
    for (auto& janitor : g_tsl_memory_janitor_stack) {
        std::shared_ptr<const AstNode> shared_ptr = janitor.find_shared_ptr(ptr);
        if (shared_ptr)
            return std::dynamic_pointer_cast<const T>(shared_ptr);
    }
    return nullptr;
}

// Instantiation with some concrete class that will be used in the compiler.
#define INSTANTIATION_AST_PTR_FROM_RAW(T) template std::shared_ptr<const T>  ast_ptr_from_raw(const AstNode* ptr);

INSTANTIATION_AST_PTR_FROM_RAW(AstNode_Expression)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_Lvalue)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_Statement)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_VariableDecl)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_FunctionBody)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_Statement_VariableDecl)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_FunctionPrototype)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_StructDeclaration)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_ArgumentList)
INSTANTIATION_AST_PTR_FROM_RAW(AstNode_Statement_StructMemberDecls)

TSL_NAMESPACE_END