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

#include <thread>
#include <algorithm>
#include "test_common.h"

TEST(Thread, Full_Test) {
    // thread number, this should be large enough to make sure it fails if the compiler is not thread safe.
    constexpr int TN = 16;

    try {
        // Unlike other unit test, this one can cause crash if it is not thread safe.
        std::vector<std::thread> threads(TN);
        for (int i = 0; i < TN; ++i)
            threads[i] = std::thread([&]() {
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
                });

        // making sure all threads are done
        std::for_each(threads.begin(), threads.end(), [](std::thread& thread) { thread.join(); });
    }
    catch (...)
    {
        // make sure it fails
        EXPECT_EQ(0, 1);
    }
}