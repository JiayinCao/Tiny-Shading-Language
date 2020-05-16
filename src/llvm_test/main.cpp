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

/*
    This is a separate project just to verifying features requested by TSL are available and functional as
    expected. If the LLVM library is not 10.0.0, this is very necessary to confirm the behavior is exactly
    the same with this version since this is what I mainly used for delopment of TSL. Any bias in term of
    LLVM behavior will likely cause unknown result.

    Also this is not a full feature requests verification, but if certain LLVM library doesn't pass through
    these unit tests, it will not be used to compile TSL.
*/
#include <iostream>
#include "gtest/gtest.h"

int main(int argc, char** argv) {
    std::cout << "-------------------------- VERIFYING LLVM FEATURES --------------------------" << std::endl;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}