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

#include "tsl_version.h"

TSL_NAMESPACE_BEGIN

// A helper class to make sure there is a temporary janitor during the life time of this guard.
class Ast_Memory_Guard {
public:
    Ast_Memory_Guard();
    ~Ast_Memory_Guard();
};

// Keep track of this AstNode. This should be called in the constructor of AstNode to secure memory leak problem.
// All ast nodes have to be allocated on heap because the smart pointers will try deleting them eventually.
// Having an ast node on stack will easily introduce a crash. Since all ast nodes are allocated by Bison script,
// they are allocated on heap instead of stack.
void ast_ptr_tracking(const class AstNode*);

// It is very important to go through this function whenver TSL manages a raw pointer with smart pointer.
// Failing to do it will result in crash. All other parts of the compiler has to convert their raw pointer input
// through this interface, which will locate the shared_ptr registered during construction.
template<class T>
std::shared_ptr<const T>    ast_ptr_from_raw(const class AstNode* ptr);

TSL_NAMESPACE_END