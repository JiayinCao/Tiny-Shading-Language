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
#include <vector>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include "tsl_define.h"
#include "ast_memory_janitor.h"
#include "compile_context.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

template<class T>
using ast_ptr = const std::shared_ptr<const T>;

class LLVM_Value{
public:
	virtual llvm::Value* codegen( Tsl_Namespace::TSL_Compile_Context& context ) const = 0;
};

class LLVM_Function{
public:
	virtual llvm::Function* codegen( TSL_Compile_Context& context ) const = 0;
};

//! @brief  Base class of ast node
/**
 * AstNode is kind of special that it can only be allocated on heap. Trying to use a AstNode on stack
 * will result in crash due to the memory management of AstNode.
 */
class AstNode {
public:
    AstNode() { ast_ptr_tracking(this); }
    virtual ~AstNode() {}

    template<class T>
    static T* castType(AstNode* node, bool check = true) {
#ifndef TSL_FINAL
        if (!node)
            return nullptr;

        T* ret = dynamic_cast<T*>(node);
        if (check)
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
};

class AstNode_Expression : public AstNode, public LLVM_Value {
public:
	virtual bool is_closure(TSL_Compile_Context& context) const { return false; }
};

class AstNode_Literal : public AstNode_Expression {

};

class AstNode_Literal_Int : public AstNode_Literal {
public:
    AstNode_Literal_Int(int val) : m_val(val) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const int m_val;

    friend class AstNode_GlobalArrayDecl;
};

class AstNode_Literal_Flt : public AstNode_Literal {
public:
    AstNode_Literal_Flt(float val) : m_val(val) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const float m_val;

    friend class AstNode_GlobalArrayDecl;
};

class AstNode_Literal_Double : public AstNode_Literal {
public:
    AstNode_Literal_Double(double val) : m_val(val) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const double m_val;
};

class AstNode_Literal_Bool: public AstNode_Literal {
public:
    AstNode_Literal_Bool(bool val) : m_val(val) {}

    llvm::Value * codegen(TSL_Compile_Context& context) const override;

private:
    const bool m_val;
};

class AstNode_Literal_GlobalValue : public AstNode_Literal {
public:
    AstNode_Literal_GlobalValue(const char* value_name) : m_value_name(value_name) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const std::string m_value_name;
};

class AstNode_Binary : public AstNode_Expression {
public:
    AstNode_Binary(AstNode_Expression* left, AstNode_Expression* right) :
        m_left(ast_ptr_from_raw<AstNode_Expression>(left)), 
        m_right(ast_ptr_from_raw<AstNode_Expression>(right)) {}

protected:
    ast_ptr<AstNode_Expression> m_left;
    ast_ptr<AstNode_Expression> m_right;
};

class AstNode_Binary_Add : public AstNode_Binary {
public:
    AstNode_Binary_Add(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    bool is_closure(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Minus : public AstNode_Binary {
public:
    AstNode_Binary_Minus(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Multi : public AstNode_Binary {
public:
    AstNode_Binary_Multi(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    bool is_closure(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Div : public AstNode_Binary {
public:
    AstNode_Binary_Div(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Mod : public AstNode_Binary {
public:
    AstNode_Binary_Mod(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_And : public AstNode_Binary {
public:
    AstNode_Binary_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Or : public AstNode_Binary {
public:
    AstNode_Binary_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Eq : public AstNode_Binary {
public:
    AstNode_Binary_Eq(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Ne : public AstNode_Binary {
public:
    AstNode_Binary_Ne(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_G : public AstNode_Binary {
public:
    AstNode_Binary_G(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_L : public AstNode_Binary {
public:
    AstNode_Binary_L(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Ge : public AstNode_Binary {
public:
    AstNode_Binary_Ge(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Le : public AstNode_Binary {
public:
    AstNode_Binary_Le(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Shl : public AstNode_Binary {
public:
    AstNode_Binary_Shl(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Shr : public AstNode_Binary {
public:
    AstNode_Binary_Shr(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Bit_And : public AstNode_Binary {
public:
    AstNode_Binary_Bit_And(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Bit_Or : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Or(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Binary_Bit_Xor : public AstNode_Binary {
public:
    AstNode_Binary_Bit_Xor(AstNode_Expression* left, AstNode_Expression* right) :AstNode_Binary(left, right) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ArgumentList : public AstNode{
public:
    AstNode_ArgumentList* add_argument(const AstNode_Expression* arg) {
        m_args.push_back(ast_ptr_from_raw<AstNode_Expression>(arg));
        return this;
    }

    const std::vector<std::shared_ptr<const AstNode_Expression>>& get_arg_list() const {
        return m_args;
    }

private:
    std::vector<std::shared_ptr<const AstNode_Expression>> m_args;
};

class AstNode_FunctionCall : public AstNode_Expression {
public:
    AstNode_FunctionCall(const char* func_name, AstNode_ArgumentList* args) :
        m_name(func_name), 
        m_args(ast_ptr_from_raw<AstNode_ArgumentList>(args)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    std::string m_name;
    ast_ptr<AstNode_ArgumentList> m_args;
};

class AstNode_Float3Constructor: public AstNode_Expression {
public:
    AstNode_Float3Constructor(AstNode_ArgumentList* variables) :
        m_arguments(ast_ptr_from_raw<AstNode_ArgumentList>(variables)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_ArgumentList> m_arguments;
};

class AstNode_Expression_MakeClosure : public AstNode_Expression {
public:
    AstNode_Expression_MakeClosure(const char* closure_name, AstNode_ArgumentList* args) :
        m_name(closure_name), 
        m_args(ast_ptr_from_raw<AstNode_ArgumentList>(args)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

	bool is_closure(TSL_Compile_Context& context) const override {
		return true;
	}

private:
    const std::string m_name;
    ast_ptr<AstNode_ArgumentList> m_args;
};

class AstNode_Ternary : public AstNode_Expression {
public:
    AstNode_Ternary(AstNode_Expression* condition, AstNode_Expression* true_exp, AstNode_Expression* false_exp) :
        m_condition(ast_ptr_from_raw<AstNode_Expression>(condition)), 
        m_true_expr(ast_ptr_from_raw<AstNode_Expression>(true_exp)), 
        m_false_expr(ast_ptr_from_raw<AstNode_Expression>(false_exp)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_condition;
    ast_ptr<AstNode_Expression> m_true_expr;
    ast_ptr<AstNode_Expression> m_false_expr;
};

class AstNode_Lvalue : public AstNode_Expression {
public:
    virtual llvm::Value* get_value_address(TSL_Compile_Context& context) const {
        return nullptr;
    }

	virtual DataType get_var_type(TSL_Compile_Context& context) const = 0;
};

class AstNode_Statement : public AstNode, public LLVM_Value {
};

class AstNode_VariableDecl : public AstNode_Statement {
public:
    virtual DataType data_type() const = 0;
    virtual const char* get_var_name() const = 0;
    virtual VariableConfig get_config() const = 0;
    virtual const AstNode_Expression* get_init() const = 0;
};

class AstNode_GlobalVariableDecl : public AstNode_VariableDecl {
};

class AstNode_SingleGlobalVariableDecl : public AstNode_GlobalVariableDecl {
public:
    AstNode_SingleGlobalVariableDecl(const char* name, const DataType type, const VariableConfig config = VariableConfig::NONE, AstNode_Expression* init_exp = nullptr)
        : m_name(name), m_type(type), m_config(config),
        m_init_exp(ast_ptr_from_raw<AstNode_Literal>(init_exp)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    DataType data_type() const override {
        return m_type;
    }

    const char* get_var_name() const override {
        return m_name.c_str();
    }

    const AstNode_Expression* get_init() const override {
        return m_init_exp.get();
    }

    VariableConfig get_config() const override {
        return m_config;
    }

private:
    const std::string		        m_name;
    const DataType			        m_type;
    const VariableConfig	        m_config;
    ast_ptr<AstNode_Expression>		m_init_exp;
};

class AstNode_SingleVariableDecl : public AstNode_VariableDecl {
public:
    AstNode_SingleVariableDecl(const char* name, const DataType type, const VariableConfig config = VariableConfig::NONE, AstNode_Expression* init_exp = nullptr)
        : m_name(name), m_type(type), m_config(config), 
        m_init_exp(ast_ptr_from_raw<AstNode_Expression>(init_exp)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

	DataType data_type() const override{
		return m_type;
	}

	const char* get_var_name() const override{
		return m_name.c_str();
	}

    const AstNode_Expression* get_init() const override{
        return m_init_exp.get();
    }

    VariableConfig get_config() const override {
        return m_config;
    }
    
private:
	const std::string		m_name;
	const DataType			m_type;
	const VariableConfig	m_config;
    ast_ptr<AstNode_Expression>		m_init_exp;
};

class AstNode_MultiVariableDecl : public AstNode {
public:
    AstNode_MultiVariableDecl* add_var(AstNode_SingleVariableDecl* var) {
        if (!var)
            return this;
        auto ptr = ast_ptr_from_raw<AstNode_SingleVariableDecl>(var);
        m_vars.push_back(ptr);
        return this;
    }

    const std::vector<std::shared_ptr<const AstNode_SingleVariableDecl>>& get_var_list() const {
        return m_vars;
    }

private:
    std::vector<std::shared_ptr<const AstNode_SingleVariableDecl>>  m_vars;
};

class AstNode_ArrayInitList : public AstNode {
public:
    AstNode_ArrayInitList* add_var(AstNode_Literal* var) {
        if (!var)
            return this;
        auto ptr = ast_ptr_from_raw<AstNode_Literal>(var);
        m_vars.push_back(ptr);
        return this;
    }

    const std::vector<std::shared_ptr<const AstNode_Literal>>& get_init_list() const {
        return m_vars;
    }

private:
    std::vector<std::shared_ptr<const AstNode_Literal>> m_vars;
};

class AstNode_GlobalArrayDecl : public AstNode_GlobalVariableDecl {
public:
    AstNode_GlobalArrayDecl(const char* name, const DataType type, AstNode_Expression* cnt, AstNode_ArrayInitList* init_list = nullptr, const VariableConfig config = VariableConfig::NONE)
        : m_name(name), m_type(type), m_config(config),
        m_cnt(ast_ptr_from_raw<AstNode_Expression>(cnt)),
        m_init(ast_ptr_from_raw< AstNode_ArrayInitList>(init_list)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    DataType data_type() const override {
        return m_type;
    }

    const char* get_var_name() const override {
        return m_name.c_str();
    }

    const AstNode_Expression* get_cnt() const {
        return m_cnt.get();
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
    ast_ptr<AstNode_Expression>     m_cnt;
    ast_ptr<AstNode_ArrayInitList>  m_init;
};

class AstNode_ArrayDecl : public AstNode_VariableDecl {
public:
    AstNode_ArrayDecl(const char* name, const DataType type, AstNode_Expression* cnt, AstNode_ArrayInitList* init_list = nullptr, const VariableConfig config = VariableConfig::NONE)
        : m_name(name), m_type(type), m_config(config), 
          m_cnt(ast_ptr_from_raw<AstNode_Expression>(cnt)),
          m_init(ast_ptr_from_raw< AstNode_ArrayInitList>(init_list)){}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    DataType data_type() const override {
        return m_type;
    }

    const char* get_var_name() const override{
        return m_name.c_str();
    }

    const AstNode_Expression* get_cnt() const {
        return m_cnt.get();
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
    ast_ptr<AstNode_Expression>     m_cnt;
    ast_ptr<AstNode_ArrayInitList>  m_init;
};

class AstNode_VariableRef : public AstNode_Lvalue {
public:
    AstNode_VariableRef(const char* name) : m_name(name) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    llvm::Value* get_value_address(TSL_Compile_Context& context) const override;

	DataType get_var_type(TSL_Compile_Context& context) const override;

    bool is_closure(TSL_Compile_Context& context) const override;

private:
	const std::string	m_name;
};

class AstNode_ArrayAccess : public AstNode_Lvalue {
public:
    AstNode_ArrayAccess(AstNode_Lvalue* var, AstNode_Expression* index) : 
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)), 
        m_index(ast_ptr_from_raw<AstNode_Expression>(index)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    llvm::Value* get_value_address(TSL_Compile_Context& context) const override;

	DataType get_var_type(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Lvalue>     m_var;
    ast_ptr<AstNode_Expression> m_index;
};

class AstNode_ExpAssign : public AstNode_Expression {
public:
    AstNode_ExpAssign(AstNode_Lvalue* var, AstNode_Expression* exp) :
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)), 
        m_expression(ast_ptr_from_raw<AstNode_Expression>(exp)) {}

protected:
    ast_ptr<AstNode_Lvalue> m_var;
    ast_ptr<AstNode_Expression> m_expression;
};

class AstNode_ExpAssign_Eq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_Eq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_AddEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AddEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_MinusEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MinusEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_MultiEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_MultiEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_DivEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_DivEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_ModEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ModEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_AndEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_AndEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_OrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_OrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_XorEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_XorEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_ShlEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShlEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_ExpAssign_ShrEq : public AstNode_ExpAssign {
public:
    AstNode_ExpAssign_ShrEq(AstNode_Lvalue* var, AstNode_Expression* exp) :AstNode_ExpAssign(var, exp) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Unary : public AstNode_Expression {
};

class AstNode_Unary_Pos : public AstNode_Expression {
public:
    AstNode_Unary_Pos(AstNode_Expression* exp) : 
        m_exp(ast_ptr_from_raw<AstNode_Expression>(exp)) {}

	llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_exp;
};

class AstNode_Unary_Neg : public AstNode_Expression {
public:
    AstNode_Unary_Neg(AstNode_Expression* exp) :
        m_exp(ast_ptr_from_raw<AstNode_Expression>(exp)) {}

	llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_exp;
};

class AstNode_Unary_Not : public AstNode_Expression {
public:
    AstNode_Unary_Not(AstNode_Expression* exp) : 
        m_exp(ast_ptr_from_raw<AstNode_Expression>(exp)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_exp;
};

class AstNode_Unary_Compl : public AstNode_Expression {
public:
    AstNode_Unary_Compl(AstNode_Expression* exp) : 
        m_exp(ast_ptr_from_raw<AstNode_Expression>(exp)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_exp;
};

class AstNode_TypeCast : public AstNode_Expression {
public:
	AstNode_TypeCast(AstNode_Expression* exp, DataType type) : 
        m_exp(ast_ptr_from_raw<AstNode_Expression>(exp)),
        m_target_type(type) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_exp;
	const DataType			    m_target_type;
};

class AstNode_Expression_PostInc : public AstNode_Expression {
public:
	AstNode_Expression_PostInc(AstNode_Lvalue* var) : 
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Lvalue> m_var;
};

class AstNode_Expression_PostDec : public AstNode_Expression {
public:
	AstNode_Expression_PostDec(AstNode_Lvalue* var):
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Lvalue> m_var;
};

class AstNode_Expression_PreInc : public AstNode_Expression {
public:
	AstNode_Expression_PreInc(AstNode_Lvalue* var) : 
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Lvalue> m_var;
};

class AstNode_Expression_PreDec : public AstNode_Expression {
public:
	AstNode_Expression_PreDec(AstNode_Lvalue* var) : 
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Lvalue> m_var;
};

class AstNode_ScoppedStatement : public AstNode_Statement {
public:
    AstNode_ScoppedStatement(AstNode_Statement* statement) :
        m_statement(ast_ptr_from_raw<AstNode_Statement>(statement)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Statement> m_statement;
};

class AstNode_CompoundStatements : public AstNode_Statement {
public:
    AstNode_CompoundStatements() {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

    void append_statement(AstNode_Statement* statement);

private:
    std::vector<std::shared_ptr<const AstNode_Statement>> m_statements;
};

class AstNode_Statement_Break : public AstNode_Statement {
public:
    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Statement_Continue : public AstNode_Statement {
public:
    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Statement_Return : public AstNode_Statement {
public:
	AstNode_Statement_Return(AstNode_Expression* expression) : 
        m_expression(ast_ptr_from_raw<AstNode_Expression>(expression)) {}
        
    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_expression;
};

class AstNode_Statement_Expression : public AstNode_Statement {
public:
	AstNode_Statement_Expression(AstNode_Expression* expression) :
        m_expression(ast_ptr_from_raw<AstNode_Expression>(expression)) {}

	llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression> m_expression;
};

class AstNode_Statement_Condition : public AstNode_Statement {
public:
	AstNode_Statement_Condition(AstNode_Expression* cond, AstNode_Statement* true_statements , AstNode_Statement* false_statements = nullptr) 
		: m_condition(ast_ptr_from_raw<AstNode_Expression>(cond)), 
          m_true_statements(ast_ptr_from_raw<AstNode_Statement>(true_statements)),
          m_false_statements(ast_ptr_from_raw<AstNode_Statement>(false_statements)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Expression>	m_condition;
    ast_ptr<AstNode_Statement>	m_true_statements;
    ast_ptr<AstNode_Statement>	m_false_statements;
};

class AstNode_Statement_VariableDecl: public AstNode_Statement {
public:
	AstNode_Statement_VariableDecl(AstNode_VariableDecl* var_decls) :
        m_var_decls(ast_ptr_from_raw<AstNode_VariableDecl>(var_decls)) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

	const AstNode_VariableDecl* get_variable_decl() const {
		return m_var_decls.get();
	}
private:
	ast_ptr<AstNode_VariableDecl> m_var_decls;
};

class AstNode_Statement_StructMemberDecls : public AstNode {
public:
    AstNode_Statement_StructMemberDecls* add_member_decl(AstNode_Statement_VariableDecl* var) {
        auto ptr = ast_ptr_from_raw<AstNode_Statement_VariableDecl>(var);
        m_members.push_back(ptr);
        return this;
    }

    const std::vector<std::shared_ptr<const AstNode_Statement_VariableDecl>>& get_member_list() const {
        return m_members;
    }

private:
    std::vector<std::shared_ptr<const AstNode_Statement_VariableDecl>> m_members;
};

class AstNode_Statement_Loop : public AstNode_Statement {
public:
	AstNode_Statement_Loop(AstNode_Expression* cond, AstNode_Statement* statements): 
        m_condition(ast_ptr_from_raw<AstNode_Expression>(cond)), 
        m_statements(ast_ptr_from_raw<AstNode_Statement>(statements)) {}

protected:
    ast_ptr<AstNode_Expression>	    m_condition;
    ast_ptr<AstNode_Statement>	    m_statements;
};

class AstNode_Statement_Loop_For : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_For(AstNode_Statement* init_exp, AstNode_Expression* cond_exp, AstNode_Expression* iter_exp, AstNode_Statement* statements):
		AstNode_Statement_Loop(cond_exp, statements), 
        m_init_exp(ast_ptr_from_raw<AstNode_Statement>(init_exp)),
        m_iter_exp(ast_ptr_from_raw<AstNode_Expression>(iter_exp)){}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    ast_ptr<AstNode_Statement>  m_init_exp;
    ast_ptr<AstNode_Expression> m_iter_exp;
};

class AstNode_Statement_Loop_While : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_While(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_Statement_Loop_DoWhile : public AstNode_Statement_Loop {
public:
	AstNode_Statement_Loop_DoWhile(AstNode_Expression* cond, AstNode_Statement* statements):AstNode_Statement_Loop(cond,statements){}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;
};

class AstNode_FunctionBody : public AstNode, LLVM_Value {
public:
	AstNode_FunctionBody(AstNode_Statement* statements) : 
        m_statements(ast_ptr_from_raw<AstNode_Statement>(statements)) {}

	llvm::Value* codegen( TSL_Compile_Context& context ) const override;

private:
    ast_ptr<AstNode_Statement>	m_statements;

    friend class AstNode_FunctionPrototype;
};

class AstNode_FunctionPrototype : public AstNode, LLVM_Function {
public:
	AstNode_FunctionPrototype(const char* func_name, AstNode_MultiVariableDecl* variables, AstNode_FunctionBody* body, bool is_shader = false, DataType type = { DataTypeEnum::VOID , nullptr } )
		                     :m_name(func_name), m_is_shader(is_shader), m_return_type(type),
                              m_variables(ast_ptr_from_raw<AstNode_MultiVariableDecl>(variables)),
                              m_body(ast_ptr_from_raw<AstNode_FunctionBody>(body)){}

	llvm::Function* codegen( TSL_Compile_Context& context ) const override;

    const std::string& get_function_name() const{
        return m_name;
    }

    void parse_shader_parameters(std::vector< ExposedArgDescriptor>& params) const;

private:
	const std::string	m_name;
    const bool          m_is_shader;
    const DataType		m_return_type;

    ast_ptr<AstNode_MultiVariableDecl>	m_variables;
    ast_ptr<AstNode_FunctionBody>       m_body;

    friend class AstNode_FunctionCall;
};

class AstNode_StructDeclaration : public AstNode, LLVM_Value {
public:
	AstNode_StructDeclaration(const char* struct_name, AstNode_Statement_StructMemberDecls* members ) :
        m_name(struct_name), 
        m_members(ast_ptr_from_raw<AstNode_Statement_StructMemberDecls>(members)) {}

	llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
	const std::string m_name;
    ast_ptr<AstNode_Statement_StructMemberDecls>	m_members;
};

class AstNode_StructMemberRef : public AstNode_Lvalue {
public:
	AstNode_StructMemberRef( AstNode_Lvalue* var , const char* member_name ) : 
        m_var(ast_ptr_from_raw<AstNode_Lvalue>(var)),
        m_member(member_name) {}

	llvm::Value* codegen(TSL_Compile_Context& context) const override;
	llvm::Value* get_value_address(TSL_Compile_Context& context) const override;

	DataType get_var_type(TSL_Compile_Context& context) const override;

private:
    const std::string	m_member;
    ast_ptr<AstNode_Lvalue>		m_var;
};

class AstNode_Statement_TextureDeclaration : public AstNode_Statement {
public:
    AstNode_Statement_TextureDeclaration(const char* texture_var_name) : 
        m_handle_name(texture_var_name) {}

    llvm::Value*    codegen(TSL_Compile_Context& context) const override;

private:
    const std::string m_handle_name;
};

class AstNode_Statement_ShaderResourceHandleDeclaration : public AstNode_Statement {
public:
    AstNode_Statement_ShaderResourceHandleDeclaration(const char* texture_var_name) : 
        m_handle_name(texture_var_name) {}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const std::string m_handle_name;
};

class AstNode_Expression_Texture2DSample : public AstNode_Expression {
public:
    AstNode_Expression_Texture2DSample(const char* texture_handle_name, AstNode_ArgumentList* variables, const bool sample_alpha = false)
        : m_texture_handle_name(texture_handle_name), m_arguments(ast_ptr_from_raw<AstNode_ArgumentList>(variables)), m_sample_alpha(sample_alpha){}

    llvm::Value* codegen(TSL_Compile_Context& context) const override;

private:
    const std::string m_texture_handle_name;
    const bool  m_sample_alpha;
    ast_ptr<AstNode_ArgumentList> m_arguments;
};

TSL_NAMESPACE_END