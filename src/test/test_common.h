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

#pragma once

#include "thirdparty/gtest/gtest.h"
#include "compiler/ast.h"

using namespace tsl;

struct yy_buffer_state;
typedef yy_buffer_state* YY_BUFFER_STATE;

int yyparse();
int yylex_destroy(void);
YY_BUFFER_STATE yy_scan_string(const char* base);
void makeVerbose(int verbose);

inline void validate_shader(const char* shader_source, bool valid = true ) {
    yy_scan_string(shader_source);
    if (valid) {
        makeVerbose(true);
        EXPECT_EQ(yyparse(), 0);
    } else {
        makeVerbose(false); // surpress the error message
        EXPECT_NE(yyparse(), 0);
    }
    yylex_destroy();
}