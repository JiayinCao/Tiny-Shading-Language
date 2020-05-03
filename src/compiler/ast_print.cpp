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

#include <iostream>
#include "ast.h"

TSL_NAMESPACE_ENTER

void AstNode_Literal_Int::print() const {
    std::cout << m_val;
}

void AstNode_Literal_Flt::print() const {
    std::cout << m_val;
}

void AstNode_Ternary::print() const {
    m_condition->print();
	std::cout << "?";
	m_true_expr->print();
	std::cout << ":";
	m_false_expr->print();
}

void AstNode_FunctionPrototype::print() const{
	std::cout << str_from_data_type(m_return_type) << " " << m_name << "(";

	bool first = true;
	AstNode* param_node = m_variables;
	while (param_node) {
		if (!first)
			std::cout << ", ";

		param_node->print();
		param_node = param_node->getSibling();

		first = false;
	}

	std::cout << ")";
}

void AstNode_FunctionDefinition::print() const {
	m_proto->print();
	m_body->print();
}

void AstNode_Shader::print() const {
	m_proto->print();
	m_body->print();
}

void AstNode_FunctionBody::print() const{
	std::cout<<"{"<<std::endl;
	
	// print the statements here
	AstNode* statement = m_statements;
	while(statement){
		statement->print();
		statement = statement->getSibling();
	}

	std::cout<<"}"<<std::endl;
}

void AstNode_FunctionCall::print() const {
    std::cout << m_name;
    std::cout << "(";

    bool first = true;
    AstNode* param_node = m_variables;
    while (param_node) {
        if (!first)
            std::cout << " , ";

        param_node->print();
        param_node = param_node->getSibling();

        first = false;
    }

    std::cout << ")";
}

void AstNode_VariableRef::print() const {
    std::cout << m_name;
}

void AstNode_VariableDecl::print() const {
	if( m_config != VariableConfig::NONE )
		std::cout<<str_from_var_config(m_config)<< " ";

	std::cout << str_from_data_type(m_type) << " " << m_name;

	if (m_init_exp) {
		std::cout << " = ";
		m_init_exp->print();
	}
}

void AstNode_VariableDecl::printVariableOnly() const{
	std::cout << m_name;

	if (m_init_exp) {
		std::cout << " = ";
		m_init_exp->print();
	}
}

void AstNode_Binary_Add::print() const {
    m_left->print();
	std::cout<< "+";
	m_right->print();
}

void AstNode_Binary_Minus::print() const {
	m_left->print();
	std::cout << "-";
	m_right->print();
}

void AstNode_Binary_Multi::print() const {
	m_left->print();
	std::cout << "*";
	m_right->print();
}

void AstNode_Binary_Div::print() const {
	m_left->print();
	std::cout << "/";
	m_right->print();
}

void AstNode_Binary_Mod::print() const {
	m_left->print();
	std::cout << "%";
	m_right->print();
}

void AstNode_Binary_And::print() const {
	m_left->print();
	std::cout << "&&";
	m_right->print();
}

void AstNode_Binary_Or::print() const {
	m_left->print();
	std::cout << "||";
	m_right->print();
}

void AstNode_Binary_Eq::print() const {
	m_left->print();
	std::cout << "==";
	m_right->print();
}

void AstNode_Binary_Ne::print() const {
	m_left->print();
	std::cout << "!=";
	m_right->print();
}

void AstNode_Binary_G::print() const {
	m_left->print();
	std::cout << ">";
	m_right->print();
}

void AstNode_Binary_L::print() const {
	m_left->print();
	std::cout << "<";
	m_right->print();
}

void AstNode_Binary_Ge::print() const {
	m_left->print();
	std::cout << ">=";
	m_right->print();
}

void AstNode_Binary_Le::print() const {
	m_left->print();
	std::cout << "<=";
	m_right->print();
}

void AstNode_Binary_Shl::print() const {
	m_left->print();
	std::cout << "<<";
	m_right->print();
}

void AstNode_Binary_Shr::print() const {
	m_left->print();
	std::cout << ">>";
	m_right->print();
}

void AstNode_Binary_Bit_And::print() const {
	m_left->print();
	std::cout << "&";
	m_right->print();
}

void AstNode_Binary_Bit_Or::print() const {
	m_left->print();
	std::cout << "|";
	m_right->print();
}

void AstNode_Binary_Bit_Xor::print() const {
	m_left->print();
	std::cout << "^";
	m_right->print();
}

void AstNode_ExpAssign_Eq::print() const {
    m_var->print();
    std::cout << "=" ;
    m_expression->print();
}

void AstNode_ExpAssign_AddEq::print() const {
    m_var->print();
    std::cout << "+=" ;
    m_expression->print();
}

void AstNode_ExpAssign_MinusEq::print() const {
    m_var->print();
    std::cout << "-=" ;
    m_expression->print();
}

void AstNode_ExpAssign_MultiEq::print() const {
    m_var->print();
    std::cout << "*=" ;
    m_expression->print();
}

void AstNode_ExpAssign_DivEq::print() const {
    m_var->print();
    std::cout << "/=" ;
    m_expression->print();
}

void AstNode_ExpAssign_ModEq::print() const {
    m_var->print();
    std::cout << "%=" ;
    m_expression->print();
}

void AstNode_ExpAssign_AndEq::print() const {
    m_var->print();
    std::cout << "&=" ;
    m_expression->print();
}

void AstNode_ExpAssign_OrEq::print() const {
    m_var->print();
    std::cout << "|=" ;
    m_expression->print();
}

void AstNode_ExpAssign_XorEq::print() const {
    m_var->print();
    std::cout << "^=" ;
    m_expression->print();
}

void AstNode_ExpAssign_ShlEq::print() const {
    m_var->print();
    std::cout << "<<=" ;
    m_expression->print();
}

void AstNode_ExpAssign_ShrEq::print() const {
    m_var->print();
    std::cout << ">>=" ;
    m_expression->print();
}

void AstNode_Unary_Pos::print() const {
    std::cout << "+";
    m_exp->print();
}

void AstNode_Unary_Neg::print() const {
    std::cout << "-";
    m_exp->print();
}

void AstNode_Unary_Not::print() const {
    std::cout << "!";
    m_exp->print();
}

void AstNode_Unary_Compl::print() const {
    std::cout << "~";
    m_exp->print();
}

void AstNode_TypeCast::print() const {
	std::cout << "(" << str_from_data_type(m_target_type) << ")";
	m_exp->print();
}

void AstNode_Expression_PostInc::print() const {
	m_exp->print();
	std::cout<<"++";
}

void AstNode_Expression_PostDec::print() const {
	m_exp->print();
	std::cout << "--";
}

void AstNode_Expression_PreInc::print() const {
	std::cout << "++";
	m_exp->print();
}

void AstNode_Expression_PreDec::print() const {
	std::cout << "--";
	m_exp->print();
}

void AstNode_Statement_Return::print() const {
	std::cout << "return";

	if( m_expression ){
		std::cout<<" ";
		m_expression->print();
	}

	std::cout<< ";" << std::endl;
}

void AstNode_Statement_CompoundExpression::print() const {
	AstNode* expression = m_expression;
	bool is_first = true;
	while(expression){
		if( !is_first )
			std::cout<<", ";
		expression->print();
		expression = expression->getSibling();

		is_first = false;
	}
	std::cout<<";"<<std::endl;
}

void AstNode_Statement_Conditinon::print() const {
	std::cout<< "if(";
	if(m_condition)
		m_condition->print();
	std::cout<< "){" << std::endl;
	if(m_true_statements)
		m_true_statements->print();
	std::cout<<"}";

	if( m_false_statements ){
		std::cout<< "else{" <<std::endl;
		m_false_statements->print();
		std::cout<<"}";
	}

	std::cout<<std::endl;
}

void AstNode_Statement_Loop_For::print() const {
	std::cout<< "for( ";
	if(m_init_exp)
		m_init_exp->print();
	std::cout<<";";
	if(m_condition)
		m_condition->print();
	std::cout<<";";
	if(m_iter_exp)
		m_condition->print();
	std::cout<< "){" << std::endl;

	if(m_statements)
		m_statements->print();

	std::cout<<"}"<<std::endl;
}

void AstNode_Statement_Loop_While::print() const{
	std::cout<<"while(";
	if(m_condition)
		m_condition->print();
	std::cout<<"){"<<std::endl;
	if(m_statements)
		m_statements->print();
	std::cout<<"}"<<std::endl;
}

void AstNode_Statement_Loop_DoWhile::print() const{
	std::cout << "do{" << std::endl;
	if (m_statements)
		m_statements->print();
	std::cout << "} while(";
	if (m_condition)
		m_condition->print();
	std::cout << ")" << std::endl;
}

void AstNode_Statement_VariableDecls::print() const {
	bool is_first = true;
	DataType type = DataType::VOID;

	AstNode_VariableDecl* variable = m_var_decls;
	while( variable ){
		if( is_first ){
			type = variable->dataType();
			std::cout<<str_from_data_type(variable->dataType())<<" ";
			is_first = false;
		}else
			std::cout<<", ";

		variable->printVariableOnly();
		variable = castType<AstNode_VariableDecl>(variable->getSibling());

		if(variable)
			assert(variable->dataType() == type);
	}

	std::cout<<";"<<std::endl;
}

TSL_NAMESPACE_LEAVE