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

TEST(Variables, Full_Test) {
    validate_shader(R"(
        int k = 0;
        float gg = 0;
        float t = 0.0, kkk = 0.0;
        shader func(){
            int k = 0;
            float gg = 0;
            // CustomData cd;

            // this is not supported for now
            // CustomData cd = CustomData();

            {
                int gG = 0;
                {
                    int g = 0;
                }
                float kga = 0.0;
            }
        }

        int after_shader = 0;
        
        // not quite sure about whether to allow this, to be decided later.
        shader second_func(){
        }

        int _this_should_work = 0;
    )");
}

TEST(Variables, Global_Variables) {
    validate_shader(R"(
        int k = 0;
        float gg = 0;
        float t = 0.0, kkk = 0.0;
        shader func(){
        }
    )");
}

TEST(Variables, Local_Variables) {
    validate_shader(R"(
        shader func(){
            int k = 0;
            float gg = 0;
            float t = 0.0, kkk = 0.0;
        }
    )");
}

TEST(Variables, Only_Global_Variables) {
    validate_shader(R"(
        int k = 0;
        float gg = 0;
        float t = 0.0, kkk = 0.0;
    )");
}

TEST(Variables, Recursive_Variables) {
    validate_shader(R"(
        shader func(){
            // data.time = 0.0;
            // data_array[0].t.da[2] = 2;
        }
    )");
}

TEST(Variables, Inc_or_Dec) {
    validate_shader(R"(
        shader func(){
            int d = 0;
            d++;
            --d;
        }
    )");
}

TEST(Variables, Invalid_Inc) {
    validate_shader(R"(
        shader func(){
            data.time++ = 0;
        }
    )", false);
}

TEST(Variables, Invalid_Dec) {
    validate_shader(R"(
        shader func(){
            --data.time = 0;
        }
    )", false );
}