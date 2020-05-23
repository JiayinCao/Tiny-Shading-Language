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
        }
    )");
}

TEST(Struct, StructureDefineRecusive) {
	validate_shader(R"(
		struct vec2 {
			float x;
			float y;
		};

		struct vec3 {
			struct vec2 xy;
			float z;
		};

        shader func(){
            struct vec3 light_dir;
        }
    )");
}

TEST(Struct, StructureAsArgument) {
	validate_shader(R"(
		struct vec2 {
			float x;
			float y;
		};

		struct vec3 {
			struct vec2 xy;
			float z;
		};

        shader func( struct vec3 light_dir , out struct vec2 output ){
        }
    )");
}