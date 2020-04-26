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

TEST(Numbers, Float_Numbers) {
    validate_shader(R"(
        shader func(){
            float t = 1.0;
            t = .0;
            t = 2.;
            t = -43.3e3;
            t = +3.e1;
            t = -.0e-2;
            t = 0.e0;
            t = .1e+0;
            t = 1.e-0;
        }
    )");
}

TEST(Numbers, Invalid_Float0 ) {
    validate_shader(R"(
        shader func(){
            int t = .e0;
        }
    )", false);
}

TEST(Numbers, Invalid_Float1 ) {
    validate_shader(R"(
        shader func(){
            int t = .1e;
        }
    )", false);
}

TEST(Numbers, Integer) {
    validate_shader(R"(
        shader func(){
            int t = 0;  // zero number
            t = -0;     // negative zero ?
            t = -1132;  // nagative number
            t = +23323; // positive number
            t = 0xaaf;  // hex number
            t = -0xaaf;  // this is actually a combination of a negate sign and a hex number
            t = +0xa9932;
        }
    )");
}

TEST(Numbers, Number_Expression) {
    validate_shader(R"(
        shader func(){
            2;
            .45;
        }
    )");
}