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
#include <unordered_map>
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "tslversion.h"
#include "types.h"

TSL_NAMESPACE_BEGIN

struct LLVM_Compile_Context{
	llvm::LLVMContext*	context = nullptr;
	llvm::Module*		module = nullptr;
	llvm::IRBuilder<>*	builder = nullptr;

    std::unordered_map<std::string, llvm::Value*>  m_var_symbols;
};

class LLVM_Value{
public:
	virtual llvm::Value* codegen( LLVM_Compile_Context& context ) const = 0;
};

class LLVM_Function{
public:
	virtual llvm::Function* codegen( LLVM_Compile_Context& context ) const = 0;
};

class AstNode {
public:
	AstNode() {}

    virtual ~AstNode() = default;

    // Append a sibling to the ast node.
    AstNode* append(AstNode* node) {
        m_next_sibling = node;
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

    AstNode* getSibling() {
        return m_next_sibling;
    }

    const AstNode* getSibling() const {
        return m_next_sibling;
    }

    // helper function to print the ast
    virtual void print() const = 0;

protected:
    AstNode* m_next_sibling = nullptr;
};

class AstNode_Expression : public AstNode, LLVM_Value {
public:
    llvm::Value* codegen(LLVM_Compile_Context& context) const override;
};

class AstNode_Literal_Int : public AstNode_Expression {
public:
    AstNode_Literal_Int(int val) : m_val(val) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    int m_val;
};

class AstNode_Literal_Flt : public AstNode_Expression {
public:
    AstNode_Literal_Flt(float val) : m_val(val) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

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

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Minus : public AstNode_Binary {
public:
    AstNode_Binary_Minus(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Multi : public AstNode_Binary {
public:
    AstNode_Binary_Multi(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Div : public AstNode_Binary {
public:
    AstNode_Binary_Div(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Mod : public AstNode_Binary {
public:
    AstNode_Binary_Mod(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_And : public AstNode_Binary {
public:
    AstNode_Binary_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Or : public AstNode_Binary {
public:
    AstNode_Binary_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Eq : public AstNode_Binary {
public:
    AstNode_Binary_Eq(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Ne : public AstNode_Binary {
public:
    AstNode_Binary_Ne(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_G : public AstNode_Binary {
public:
    AstNode_Binary_G(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_L : public AstNode_Binary {
public:
    AstNode_Binary_L(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_Ge : public AstNode_Binary {
public:
    AstNode_Binary_Ge(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_Le : public AstNode_Binary {
public:
    AstNode_Binary_Le(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_Shl : public AstNode_Binary {
public:
    AstNode_Binary_Shl(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_Shr : public AstNode_Binary {
public:
    AstNode_Binary_Shr(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    void print() const override;
};

class AstNode_Binary_Bit_And : public AstNode_Binary {
public:
    AstNode_Binary_Bit_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Bit_Or : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Bit_Xor : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Xor(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_FunctionCall : public AstNode_Expression {
public:
    AstNode_FunctionCall(const char* func_name, AstNode_Expression* variables) : m_name(func_name), m_variables(variables) {}

    void print() const override;

private:
    std::string m_name;
    AstNode_Expression* m_variables;
};

class AstNode_Ternary : public AstNode_Expression {
public:
    AstNode_Ternary(AstNode_Expression* condition, AstNode_Expression* true_exp, AstNode_Expression* false_exp) :m_condition(condition), m_true_expr(true_exp), m_false_expr(false_exp) {}

    void print() const override;

private:
    AstNode_Expression* m_condition;
    AstNode_Expression* m_true_expr;
    AstNode_Expression* m_false_expr;
};

class AstNode_Lvalue : public AstNode_Expression {

};

class AstNode_VariableDecl : public AstNode {
public:
	AstNode_VariableDecl(const char* name, const DataType type, const VariableConfig config = VariableConfig::NONE, AstNode_Expression* init_exp = nullptr) 
		: m_name(name), m_type(type), m_config(config), m_init_exp(init_exp) {}

	void print() const override;

	void printVariableOnly() const;

	DataType dataType() const{
		return m_type;
	}

	const char* get_var_name() const {
		return m_name.c_str();
	}

    const AstNode_Expression* get_init() const{
        return m_init_exp;
    }
    
private:
	const std::string		m_name;
	const DataType			m_type;
	const VariableConfig	m_config;
	AstNode_Expression*		m_init_exp;
};

class AstNode_VariableRef : public AstNode_Lvalue {
public:
    AstNode_VariableRef(const char* name) : m_name(name) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
	const std::string	m_name;
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

    void print() const override;
};

class AstNode_ExpAssign_AddEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AddEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_MinusEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MinusEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_MultiEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MultiEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_DivEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_DivEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_ModEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ModEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_AndEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AndEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_OrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_OrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_XorEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_XorEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_ShlEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShlEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_ExpAssign_ShrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    void print() const override;
};

class AstNode_Unary : public AstNode_Expression {
};

class AstNode_Unary_Pos : public AstNode_Expression {
public:
    AstNode_Unary_Pos(AstNode_Expression* exp) : m_exp(exp) {}

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Neg : public AstNode_Expression {
public:
    AstNode_Unary_Neg(AstNode_Expression* exp) : m_exp(exp) {}

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Not : public AstNode_Expression {
public:
    AstNode_Unary_Not(AstNode_Expression* exp) : m_exp(exp) {}

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Compl : public AstNode_Expression {
public:
    AstNode_Unary_Compl(AstNode_Expression* exp) : m_exp(exp) {}

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_TypeCast : public AstNode_Expression {
public:
	AstNode_TypeCast(AstNode_Expression* exp, DataType type) : m_exp(exp), m_target_type(type) {}

	void print() const override;

private:
	AstNode_Expression* m_exp;
	DataType			m_target_type;
};

class AstNode_Expression_PostInc : public AstNode_Expression {
public:
	AstNode_Expression_PostInc(AstNode_Expression* exp) : m_exp(exp){}

	void print() const override;

private:
	AstNode_Expression* m_exp;
};

class AstNode_Expression_PostDec : public AstNode_Expression {
public:
	AstNode_Expression_PostDec(AstNode_Expression* exp) : m_exp(exp) {}

	void print() const override;

private:
	AstNode_Expression* m_exp;
};

class AstNode_Expression_PreInc : public AstNode_Expression {
public:
	AstNode_Expression_PreInc(AstNode_Expression* exp) : m_exp(exp) {}

	void print() const override;

private:
	AstNode_Expression* m_exp;
};

class AstNode_Expression_PreDec : public AstNode_Expression {
public:
	AstNode_Expression_PreDec(AstNode_Expression* exp) : m_exp(exp) {}

	void print() const override;

private:
	AstNode_Expression* m_exp;
};


class AstNode_Statement : public AstNode, LLVM_Value {
public:
    // this is to be deleted once all statements have this implementation
    llvm::Value* codegen(LLVM_Compile_Context& context) const override {
        return nullptr;
    }
};

class AstNode_Statement_Return : public AstNode_Statement {
public:
	AstNode_Statement_Return(AstNode_Expression* expression) : m_expression(expression) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
	AstNode_Expression* m_expression;
};

class AstNode_Statement_CompoundExpression : public AstNode_Statement {
public:
	AstNode_Statement_CompoundExpression(AstNode_Expression* expression) : m_expression(expression) {}

	void print() const override;

private:
	AstNode_Expression* m_expression;
};

class AstNode_Statement_Conditinon : public AstNode_Statement {
public:
	AstNode_Statement_Conditinon(AstNode_Expression* cond, AstNode_Statement* true_statements , AstNode_Statement* false_statements = nullptr) 
		: m_condition(cond), m_true_statements(true_statements), m_false_statements(false_statements) {}

	void print() const override;

private:
	AstNode_Expression*	m_condition;
	AstNode_Statement*	m_true_statements;
	AstNode_Statement*	m_false_statements;
};

class AstNode_Statement_VariableDecls: public AstNode_Statement {
public:
	AstNode_Statement_VariableDecls(AstNode_VariableDecl* var_decls) :m_var_decls(var_decls) {}

	void print() const override;

private:
	AstNode_VariableDecl* m_var_decls;
};

class AstNode_Statement_Loop : public AstNode_Statement {
public:
	AstNode_Statement_Loop(AstNode_Expression* cond, AstNode_Statement* statements): m_condition(cond), m_statements(statements) {}

protected:
	AstNode_Expression*	m_condition;
	AstNode_Statement*	m_statements;
};

class AstNode_Statement_Loop_For : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_For(AstNode_Expression* init_exp, AstNode_Expression* cond_exp, AstNode_Expression* iter_exp, AstNode_Statement* statements):
		AstNode_Statement_Loop(cond_exp, statements), m_init_exp(init_exp), m_iter_exp(iter_exp){}

	void print() const override;

private:
	AstNode_Expression* m_init_exp;
	AstNode_Expression* m_iter_exp;
};

class AstNode_Statement_Loop_While : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_While(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

	void print() const override;
};

class AstNode_Statement_Loop_DoWhile : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_DoWhile(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

	void print() const override; 
};

class AstNode_FunctionBody : public AstNode, LLVM_Value {
public:
	AstNode_FunctionBody(AstNode_Statement* statements) : m_statements(statements) {}

	llvm::Value* codegen( LLVM_Compile_Context& context ) const override;

	void print() const override;

private:
	AstNode_Statement*	m_statements;

    // allow prototype to access its private data
    friend class AstNode_FunctionPrototype;
};

class AstNode_FunctionPrototype : public AstNode, LLVM_Function {
public:
	AstNode_FunctionPrototype(const char* func_name, AstNode_VariableDecl* variables, 
                              AstNode_FunctionBody* body, bool is_shader = false, DataType type = DataType::VOID)
		                     :m_name(func_name), m_variables(variables), m_body(body), m_is_shader(is_shader), m_return_type(type){}

	llvm::Function* codegen( LLVM_Compile_Context& context ) const override;

	void print() const override;

private:
	const std::string		m_name;
	AstNode_VariableDecl*	m_variables;
    AstNode_FunctionBody*   m_body;
    const bool              m_is_shader;
	DataType				m_return_type;
};

TSL_NAMESPACE_END