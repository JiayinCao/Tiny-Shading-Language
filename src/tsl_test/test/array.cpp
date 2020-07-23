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

TEST(Array, Initializer) {
    auto shader_source = R"(
        shader function_name(out float var){
            float a[2] = { 1.0f, 5.0f };
            var = a[1];
        }
    )";

    auto ret = compile_shader<void(*)(float*)>(shader_source);
    auto func_ptr = ret.first;

    float data = 0.0f;
    func_ptr(&data);
    EXPECT_EQ(5.0f, data);
}
