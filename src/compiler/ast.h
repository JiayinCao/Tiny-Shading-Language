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
    AstNode* append(const AstNode* node) {
        next_sibling = node;
        return this;
    }
    
    template<class T>
    static T* castType(AstNode* node) {
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
    static const T* castType(const AstNode* node) {
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

class AstNode_Expression : public AstNode {

};

class AstNode_Literal_Int : public AstNode_Expression {
public:
    AstNode_Literal_Int(int val) : m_val(val) {}

private:
    int m_val;
};

class AstNode_Literal_Flt : public AstNode_Expression {
public:
    AstNode_Literal_Flt(float val) : m_val(val) {}

private:
    float m_val;
};

class AstNode_Binary : public AstNode_Expression {
public:
    AstNode_Binary(AstNode_Expression* left, AstNode_Expression* right) :m_left(left), m_right(right) {}

protected:
    AstNode_Expression* m_left;
    AstNode_Expression* m_right;
};

class AstNode_Binary_Add : public AstNode_Binary {
public:
    AstNode_Binary_Add(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Minus : public AstNode_Binary {
public:
    AstNode_Binary_Minus(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Multi : public AstNode_Binary {
public:
    AstNode_Binary_Multi(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Div : public AstNode_Binary {
public:
    AstNode_Binary_Div(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Mod : public AstNode_Binary {
public:
    AstNode_Binary_Mod(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_And : public AstNode_Binary {
public:
    AstNode_Binary_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Or : public AstNode_Binary {
public:
    AstNode_Binary_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Eq : public AstNode_Binary {
public:
    AstNode_Binary_Eq(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Ne : public AstNode_Binary {
public:
    AstNode_Binary_Ne(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_G : public AstNode_Binary {
public:
    AstNode_Binary_G(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_L : public AstNode_Binary {
public:
    AstNode_Binary_L(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Ge : public AstNode_Binary {
public:
    AstNode_Binary_Ge(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Le : public AstNode_Binary {
public:
    AstNode_Binary_Le(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Shl : public AstNode_Binary {
public:
    AstNode_Binary_Shl(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Shr : public AstNode_Binary {
public:
    AstNode_Binary_Shr(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Bit_And : public AstNode_Binary {
public:
    AstNode_Binary_Bit_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Bit_Or : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};

class AstNode_Binary_Bit_Xor : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Xor(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}
};


class AstNode_FunctionCall : public AstNode_Expression {
public:
    AstNode_FunctionCall(const char* func_name, AstNode_Expression* variables) : m_name(func_name), m_variables(variables) {}

private:
    std::string m_name;
    AstNode_Expression* m_variables;
};

class AstNode_Ternary : public AstNode_Expression {
public:
    AstNode_Ternary(AstNode_Expression* condition, AstNode_Expression* true_exp, AstNode_Expression* false_exp) :m_condition(condition), m_true_expr(true_exp), m_false_expr(false_exp) {}

private:
    AstNode_Expression* m_condition;
    AstNode_Expression* m_true_expr;
    AstNode_Expression* m_false_expr;
};

class AstNode_Lvalue : public AstNode_Expression {

};

class AstNode_VariableRef : public AstNode_Lvalue {
public:
    AstNode_VariableRef(const char* name) : m_name(name) {}

private:
    std::string m_name;
};

class AstNode_Function : public AstNode {
public:
    AstNode_Function(const char* func_name, const AstNode_VariableRef* variables) : m_name(func_name), m_variables(variables) {}

private:
    std::string m_name;
    const AstNode_VariableRef* m_variables;
};

class AstNode_ExpAssign : public AstNode_Expression {
public:
    AstNode_ExpAssign(AstNode_Lvalue* var, AstNode_Expression* exp) :m_var(var), m_expression(exp) {}

protected:
    AstNode_Lvalue* m_var;
    AstNode_Expression* m_expression;
};

class AstNode_ExpAssign_Eq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_Eq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_AddEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AddEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_MinusEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MinusEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_MultiEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MultiEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_DivEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_DivEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_ModEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ModEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_AndEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AndEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_OrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_OrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_XorEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_XorEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_ShlEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShlEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_ExpAssign_ShrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}
};

class AstNode_Unary : public AstNode_Expression {
};

class AstNode_Unary_Pos : public AstNode_Expression {
public:
    AstNode_Unary_Pos(AstNode_Expression* exp) : m_exp(exp) {}

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Neg : public AstNode_Expression {
public:
    AstNode_Unary_Neg(AstNode_Expression* exp) : m_exp(exp) {}

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Not : public AstNode_Expression {
public:
    AstNode_Unary_Not(AstNode_Expression* exp) : m_exp(exp) {}

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Compl : public AstNode_Expression {
public:
    AstNode_Unary_Compl(AstNode_Expression* exp) : m_exp(exp) {}

private:
    AstNode_Expression* m_exp;
};

TSL_NAMESPACE_LEAVE