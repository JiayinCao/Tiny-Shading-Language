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

// In an ideal world, I should have switched to smart pointer solution in Bison code so that it is automatically guaranteed to be memory leak free.
// However, TSL uses C version bison instead of C++. This is because the lack of pre-compiled bison with a version higher than 2.7.
// It is also possible to compile the bison source code on Cygwin to run it on Windows, but it will make CI/CD a lot more complex than what is originally
// necessary.
// To make sure there is no memory leak, this file provides interface to make sure no AST node can survive if the shader unit is deallocated to secure 
// memory leak problem. It is not the ideal solution, but I don't have time to investigate a better solution unless there is any big problem of this solution.

#pragma once

#include "tslversion.h"

TSL_NAMESPACE_BEGIN

//! @brief  A helper class to make sure there is a temporary janitor during the life time of this guard.
class Ast_Memory_Guard {
public:
    Ast_Memory_Guard();
    ~Ast_Memory_Guard();
};

class AstNode;

template<class T>
using ast_ptr = const std::shared_ptr<const T>;

//! @brief  Keep track of this AstNode
void track_ast_node(const AstNode*);

// It is very important to go through this function whenver TSL manages a raw pointer with smart pointer.
// Failing to do it will result in crash.
template<class T>
std::shared_ptr<const T>    ast_ptr_from_raw(const AstNode* ptr);

// just a macro to hide the details
#define AST_MEMORY_GUARD        Ast_Memory_Guard ast_memory_guard;

TSL_NAMESPACE_END