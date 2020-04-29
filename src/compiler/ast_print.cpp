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

void AstNode_Shader::print() const {
    std::cout << "shader " << m_name;
    std::cout << "(";

    /*
    bool first = true;
    AstNode* param_node = m_variables;
    while (param_node) {
        param_node->print();

        if (!first)
            std::cout << " , ";

        param_node = param_node->getSibling();

        first = false;
    }
    */

    std::cout << ")" << std::endl;
}

void AstNode_Ternary::print() const {
    std::cout << m_condition << "?" << m_true_expr << ":" << m_false_expr;
}

void AstNode_Function::print() const {
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

    std::cout << ")" << std::endl;
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

    std::cout << ")" << std::endl;
}

void AstNode_VariableRef::print() const {
    std::cout << m_name;
}

void AstNode_Binary_Add::print() const {
    std::cout << m_left << "+" << m_right;
}

void AstNode_Binary_Minus::print() const {
    std::cout << m_left << "-" << m_right;
}

void AstNode_Binary_Multi::print() const {
    std::cout << m_left << "*" << m_right;
}

void AstNode_Binary_Div::print() const {
    std::cout << m_left << "/" << m_right;
}

void AstNode_Binary_Mod::print() const {
    std::cout << m_left << "%" << m_right;
}

void AstNode_Binary_And::print() const {
    std::cout << m_left << "&&" << m_right;
}

void AstNode_Binary_Or::print() const {
    std::cout << m_left << "||" << m_right;
}

void AstNode_Binary_Eq::print() const {
    std::cout << m_left << "==" << m_right;
}

void AstNode_Binary_Ne::print() const {
    std::cout << m_left << "!=" << m_right;
}

void AstNode_Binary_G::print() const {
    std::cout << m_left << ">" << m_right;
}

void AstNode_Binary_L::print() const {
    std::cout << m_left << "<" << m_right;
}

void AstNode_Binary_Ge::print() const {
    std::cout << m_left << ">=" << m_right;
}

void AstNode_Binary_Le::print() const {
    std::cout << m_left << "<=" << m_right;
}

void AstNode_Binary_Shl::print() const {
    std::cout << m_left << "<<" << m_right;
}

void AstNode_Binary_Shr::print() const {
    std::cout << m_left << ">>" << m_right;
}

void AstNode_Binary_Bit_And::print() const {
    std::cout << m_left << "&" << m_right;
}

void AstNode_Binary_Bit_Or::print() const {
    std::cout << m_left << "|" << m_right;
}

void AstNode_Binary_Bit_Xor::print() const {
    std::cout << m_left << "^" << m_right;
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
    std::cout << "//=" ;
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

TSL_NAMESPACE_LEAVE