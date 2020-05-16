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

TEST(Comments, Full_Test){
    validate_shader(R"(
        /*
           this is some random comments.
        */
        shader /* I'm everywhere! */ function_name( /* just giv eit a try */ )
        {
            // /* this should be ignored.
            //* this should be valid

            // This is an ugly line that is full of comments, but it is a valid one.
            /* start from here */ int /* I'm here. */ k /* I'm also here. */ = /* Here again. */ 0 /* again */;

            int kk = 0; // this should be fine too.
        }

        /* I'm not a blocker. // */

        /* /* This is valid. */
    )");
}

TEST(Comments, Invalid_Comment0){
    validate_shader(R"(
        shader function_name(){
            /*/
        }
    )", false);
}

TEST(Comments, Invalid_Comment1) {
    validate_shader(R"(
        shader function_name(){
            /* this is right for now. */ this is so wrong! */
        }
    )", false);
}

TEST(Comments, Invalid_Comment2) {
    validate_shader(R"(
        shader function_name( // ){
        }
    )", false);
}

TEST(Comments, Invalid_Comment3) {
    validate_shader(R"(
        shader functio/*this should be treated as an error*/n_name(){
        }
    )", false);
}

TEST(Comments, Invalid_Comment4) {
    validate_shader(R"(
        shader function_name(){

            /* // */ this is not correct.
        }
    )", false);
}