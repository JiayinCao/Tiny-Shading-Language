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
#include <stack>
#include <unordered_map>
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "tslversion.h"
#include "types.h"

TSL_NAMESPACE_BEGIN

class AstNode_FunctionPrototype;

struct LLVM_Compile_Context{
	llvm::LLVMContext*	context = nullptr;
	llvm::Module*		module = nullptr;
	llvm::IRBuilder<>*	builder = nullptr;

    std::unordered_map<std::string, llvm::Value*>       m_var_symbols;
    std::unordered_map<std::string, std::pair<llvm::Function*, const AstNode_FunctionPrototype*>>    m_func_symbols;

    std::stack<std::pair<llvm::BasicBlock*, llvm::BasicBlock*>>  m_blocks;
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

    AstNode* get_sibling() {
        return m_next_sibling;
    }

    const AstNode* get_sibling() const {
        return m_next_sibling;
    }

    // helper function to print the ast
    virtual void print() const = 0;

protected:
    AstNode* m_next_sibling = nullptr;
};

class AstNode_Expression : public AstNode, public LLVM_Value {
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

class AstNode_Literal_Bool: public AstNode_Expression{
public:
    AstNode_Literal_Bool(bool val) : m_val(val) {}

    llvm::Value * codegen(LLVM_Compile_Context & context) const override;

    void print() const override;

private:
    bool m_val;
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

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_G : public AstNode_Binary {
public:
    AstNode_Binary_G(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_L : public AstNode_Binary {
public:
    AstNode_Binary_L(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Ge : public AstNode_Binary {
public:
    AstNode_Binary_Ge(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Le : public AstNode_Binary {
public:
    AstNode_Binary_Le(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Shl : public AstNode_Binary {
public:
    AstNode_Binary_Shl(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Binary_Shr : public AstNode_Binary {
public:
    AstNode_Binary_Shr(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

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

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    std::string m_name;
    AstNode_Expression* m_variables;
};

class AstNode_Ternary : public AstNode_Expression {
public:
    AstNode_Ternary(AstNode_Expression* condition, AstNode_Expression* true_exp, AstNode_Expression* false_exp) :m_condition(condition), m_true_expr(true_exp), m_false_expr(false_exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Expression* m_condition;
    AstNode_Expression* m_true_expr;
    AstNode_Expression* m_false_expr;
};

class AstNode_Lvalue : public AstNode_Expression {
public:
    virtual llvm::Value* get_value_address(LLVM_Compile_Context& context) const {
        return nullptr;
    }
};

class AstNode_VariableDecl : public AstNode, public LLVM_Value {
public:
    virtual DataType data_type() const = 0;
    virtual const char* get_var_name() const = 0;
    virtual VariableConfig get_config() const = 0;
    virtual const AstNode_Expression* get_init() const = 0;
    virtual void printVariableOnly() const = 0;
};

class AstNode_SingleVariableDecl : public AstNode_VariableDecl {
public:
    AstNode_SingleVariableDecl(const char* name, const DataType type, const VariableConfig config = VariableConfig::NONE, AstNode_Expression* init_exp = nullptr)
		: m_name(name), m_type(type), m_config(config), m_init_exp(init_exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

	void printVariableOnly() const override;

	DataType data_type() const override{
		return m_type;
	}

	const char* get_var_name() const override{
		return m_name.c_str();
	}

    const AstNode_Expression* get_init() const override{
        return m_init_exp;
    }

    VariableConfig get_config() const override {
        return m_config;
    }
    
private:
	const std::string		m_name;
	const DataType			m_type;
	const VariableConfig	m_config;
	AstNode_Expression*		m_init_exp;
};

class AstNode_ArrayDecl : public AstNode_VariableDecl {
public:
    AstNode_ArrayDecl(const char* name, const DataType type, AstNode_Expression* cnt, const VariableConfig config = VariableConfig::NONE)
        : m_name(name), m_type(type), m_config(config), m_cnt(cnt) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
    void printVariableOnly() const override;

    DataType data_type() const override {
        return m_type;
    }

    const char* get_var_name() const override{
        return m_name.c_str();
    }

    const AstNode_Expression* get_cnt() const {
        return m_cnt;
    }

    VariableConfig get_config() const override {
        return m_config;
    }

    const AstNode_Expression* get_init() const override {
        // no support for now
        return nullptr;
    }

private:
    const std::string		m_name;
    const DataType			m_type;
    const VariableConfig	m_config;
    AstNode_Expression*     m_cnt;
};

class AstNode_VariableRef : public AstNode_Lvalue {
public:
    AstNode_VariableRef(const char* name) : m_name(name) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    llvm::Value* get_value_address(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
	const std::string	m_name;
};

class AstNode_ArrayAccess : public AstNode_Lvalue {
public:
    AstNode_ArrayAccess(AstNode_Lvalue* var, AstNode_Expression* index) : m_var(var), m_index(index) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    llvm::Value* get_value_address(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Lvalue*     m_var;
    AstNode_Expression* m_index;
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

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_AddEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AddEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_MinusEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MinusEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_MultiEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MultiEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_DivEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_DivEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_ModEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ModEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_AndEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AndEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_OrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_OrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_XorEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_XorEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_ShlEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShlEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_ExpAssign_ShrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Unary : public AstNode_Expression {
};

class AstNode_Unary_Pos : public AstNode_Expression {
public:
    AstNode_Unary_Pos(AstNode_Expression* exp) : m_exp(exp) {}

	llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Neg : public AstNode_Expression {
public:
    AstNode_Unary_Neg(AstNode_Expression* exp) : m_exp(exp) {}

	llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Not : public AstNode_Expression {
public:
    AstNode_Unary_Not(AstNode_Expression* exp) : m_exp(exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_Unary_Compl : public AstNode_Expression {
public:
    AstNode_Unary_Compl(AstNode_Expression* exp) : m_exp(exp) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;

private:
    AstNode_Expression* m_exp;
};

class AstNode_TypeCast : public AstNode_Expression {
public:
	AstNode_TypeCast(AstNode_Expression* exp, DataType type) : m_exp(exp), m_target_type(type) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
	AstNode_Expression* m_exp;
	DataType			m_target_type;
};

class AstNode_Expression_PostInc : public AstNode_Expression {
public:
	AstNode_Expression_PostInc(AstNode_Lvalue* var) : m_var(var){}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
    AstNode_Lvalue* m_var;
};

class AstNode_Expression_PostDec : public AstNode_Expression {
public:
	AstNode_Expression_PostDec(AstNode_Lvalue* var) : m_var(var) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
    AstNode_Lvalue* m_var;
};

class AstNode_Expression_PreInc : public AstNode_Expression {
public:
	AstNode_Expression_PreInc(AstNode_Lvalue* var) : m_var(var) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
    AstNode_Lvalue* m_var;
};

class AstNode_Expression_PreDec : public AstNode_Expression {
public:
	AstNode_Expression_PreDec(AstNode_Lvalue* var) : m_var(var) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
    AstNode_Lvalue* m_var;
};


class AstNode_Statement : public AstNode, public LLVM_Value {
};

class AstNode_Stament_Break : public AstNode_Statement {
public:
    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
};

class AstNode_Stament_Continue : public AstNode_Statement {
public:
    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

    void print() const override;
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

	llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
	AstNode_Expression* m_expression;
};

class AstNode_Statement_Condition : public AstNode_Statement {
public:
	AstNode_Statement_Condition(AstNode_Expression* cond, AstNode_Statement* true_statements , AstNode_Statement* false_statements = nullptr) 
		: m_condition(cond), m_true_statements(true_statements), m_false_statements(false_statements) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
	AstNode_Expression*	m_condition;
	AstNode_Statement*	m_true_statements;
	AstNode_Statement*	m_false_statements;
};

class AstNode_Statement_VariableDecls: public AstNode_Statement {
public:
	AstNode_Statement_VariableDecls(AstNode_VariableDecl* var_decls) :m_var_decls(var_decls) {}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

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
	AstNode_Statement_Loop_For(AstNode_Statement* init_exp, AstNode_Expression* cond_exp, AstNode_Expression* iter_exp, AstNode_Statement* statements):
		AstNode_Statement_Loop(cond_exp, statements), m_init_exp(init_exp), m_iter_exp(iter_exp){}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;

private:
    AstNode_Statement* m_init_exp;
	AstNode_Expression* m_iter_exp;
};

class AstNode_Statement_Loop_While : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_While(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override;
};

class AstNode_Statement_Loop_DoWhile : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_DoWhile(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

    llvm::Value* codegen(LLVM_Compile_Context& context) const override;

	void print() const override; 
};

class AstNode_FunctionBody : public AstNode, LLVM_Value {
public:
	AstNode_FunctionBody(AstNode_Statement* statements) : m_statements(statements) {}

	llvm::Value* codegen( LLVM_Compile_Context& context ) const override;

	void print() const override;

private:
	AstNode_Statement*	m_statements;

    friend class AstNode_FunctionPrototype;
};

class AstNode_FunctionPrototype : public AstNode, LLVM_Function {
public:
	AstNode_FunctionPrototype(const char* func_name, AstNode_VariableDecl* variables, 
                              AstNode_FunctionBody* body, bool is_shader = false, DataType type = DataType::VOID)
		                     :m_name(func_name), m_variables(variables), m_body(body), m_is_shader(is_shader), m_return_type(type){}

	llvm::Function* codegen( LLVM_Compile_Context& context ) const override;

    const std::string& get_function_name() const{
        return m_name;
    }

	void print() const override;

private:
	const std::string		m_name;
	AstNode_VariableDecl*	m_variables;
    AstNode_FunctionBody*   m_body;
    const bool              m_is_shader;
	DataType				m_return_type;

    friend class AstNode_FunctionCall;
};

TSL_NAMESPACE_END