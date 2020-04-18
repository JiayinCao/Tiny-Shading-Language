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

TEST(Logic, Full_Test) {
    validate_shader(R"(
        shader func(){
            int flag = 1;

            if( flag ){
                if( flag2 )
                    flag = false;
                int test = 0;
            }

            if( !flag ){
            }else

            {
                int k = 0;
            }
        }
    )");
}

TEST(Logic, Compound_Condition) {
    validate_shader(R"(
        shader func(){
            int flag = 1;

            if( flag , asfdf ){
                int test = 0;
            }
        }
    )");
}

TEST(Logic, While_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            while( k++ < 100 ){
                k += 1;
                ++k;
            }
        }
    )");
}

TEST(Logic, Do_While_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            do{
                k += 1;
                ++k;
            }while( k++ < 100 );

            do
                k += 1;
            while( k++ < 100 );
        }
    )");
}

TEST(Logic, For_Loop) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            for( ; k < 100 ; ++k ){
                k += 2;
            }

            for( k = 2 ; k < 100 ; )
                ++k;
  
            for( k = 1 ; k < 100 ; ++k )
                ++k;
        }
    )");
}