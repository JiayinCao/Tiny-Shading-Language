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

#pragma once

#include <assert.h>
#include <string>
#include "include/tslversion.h"

TSL_NAMESPACE_ENTER

class AstNode {
public:
    virtual ~AstNode() = default;

    // Append a sibling to the ast node.
    AstNode* Append(const AstNode* node) {
        next_sibling = node;
        return this;
    }
    
    template<class T>
    static T* CastType(AstNode* node) {
#ifndef TSL_FINAL
        if (!node)
            return nullptr;

        T* ret = dynamic_cast<T*>(node);
        assert(ret);
        return ret;
#else
        // there is no need to pay run-time cost since we are sure the type is correct.
        return (T*)node;
#endif
    }

    template<class T>
    static const T* CastType(const AstNode* node) {
#ifndef TSL_FINAL
        if (!node)
            return nullptr;

        const T* ret = dynamic_cast<const T*>(node);
        assert(ret);
        return ret;
#else
        // there is no need to pay run-time cost since we are sure the type is correct.
        return (const T*)node;
#endif
    }

protected:
    const AstNode* next_sibling = nullptr;
};

class AstNode_Shader : public AstNode {
public:
    AstNode_Shader(const char* func_name) : name(func_name) {}

private:
    std::string name;
};

class AstNode_Variable : public AstNode {
public:
    AstNode_Variable(const char* name) : var_name(name) {}

protected:
    std::string var_name;
};

class AstNode_Function : public AstNode {
public:
    AstNode_Function(const char* func_name, const AstNode_Variable* variables) : name(func_name), variables(variables) {
    }

private:
    std::string name;
    const AstNode_Variable* variables;
};

TSL_NAMESPACE_LEAVE