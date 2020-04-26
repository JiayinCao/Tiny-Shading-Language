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

#include "test_common.h"

// Ast node is not supposed to be exposed to external compiler code, I need to figure out ways later.

TEST(AST, DISABLED_SingleParameter_Function) {
    /*
    Tsl_Scanner scanner;
    validate_shader(R"(
        int func( int catch_my_name = 0 ){
        }
    )" , true , &scanner);

    EXPECT_NE(scanner.root, nullptr);

    const AstNode_Function* func = dynamic_cast<const AstNode_Function*>(scanner.root);
    EXPECT_NE(func, nullptr);
    */
}

TEST(AST, DISABLED_Parameters_Function) {
    /*
    Tsl_Scanner scanner;
    validate_shader(R"(
        int func( int catch_my_name = 0 , int gg = 0 ){
        }
    )", true, &scanner);

    EXPECT_NE(scanner.root, nullptr);

    const AstNode_Function* func = dynamic_cast<const AstNode_Function*>(scanner.root);
    EXPECT_NE(func, nullptr);
    */
}