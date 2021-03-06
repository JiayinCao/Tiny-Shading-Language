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

TEST(Struct, StructureDefine) {
    validate_shader(R"(
        struct vec3 {
            float x;
            float y;
            float z;
        };

        shader func(){
            struct vec3 light_dir;
            light_dir.x = 2.0;
        }
    )");
}

TEST(Struct, StructureDefineRecusive) {
    const auto shader_source = R"(
        struct vec2 {
            float x;
            float y;
        };

        struct vec3 {
            struct vec2 xy;
            float z;
        };

        struct vec2 test(){
            struct vec2 t;
            t.y = 1233.0;
            t.x = 0.0;
            return t;
        }

        void helper( out struct vec2 v ){
            v = test();
        }

        shader func( out struct vec3 light_dir ){
            light_dir.z = 123.0;
            helper( light_dir.xy );
            light_dir.xy.x = 111.0;
        }
    )";

    struct vec2 {
        float x, y;
    };
    struct vec3 {
        vec2 xy;
        float z;
    };
    auto ret = compile_shader<void(*)(vec3*)>(shader_source);
    auto func_ptr = ret.first;

    vec3 v;
    func_ptr(&v);
    EXPECT_EQ(111.0f, v.xy.x);
    EXPECT_EQ(1233.0f, v.xy.y);
    EXPECT_EQ(123.0f, v.z);
}

TEST(Struct, StructureAsArgument) {
    const auto shader_source = R"(
        struct vec2 {
            float x;
            float y;
        };

        struct vec3 {
            struct vec2 xy;
            float z;
        };

        void internal_helper( out struct vec2 output ){
            output.y = 123.0;
        }

        shader func( out struct vec2 output ){
            output.x = 3123.0;
            internal_helper( output );
        }
    )";

    struct vec2{
        float x, y;
    };
    auto ret = compile_shader<void(*)(vec2*)>(shader_source);
    auto func_ptr = ret.first;

    vec2 v;
    func_ptr(&v);
    EXPECT_EQ(3123.0f, v.x);
    EXPECT_EQ(123.0f, v.y);
}

TEST(Struct, IntrinsicDataStructure) {
    const auto shader_source = R"(
        shader func( out vector output ){
            output.x = 3123.0;
            output.g = 12.0;
            output.z = 23.0;
        }
    )";

    struct float3 {
        float x, y, z;
    };
    auto ret = compile_shader<void(*)(float3*)>(shader_source);
    auto func_ptr = ret.first;

    float3 v;
    func_ptr(&v);
    EXPECT_EQ(3123.0f, v.x);
    EXPECT_EQ(12.0f, v.y);
    EXPECT_EQ(23.0f, v.z);
}