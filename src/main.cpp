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

#if 0

// this is just for verification purpose on Mac and Ubunt, this will be removed shortly after I configure LLVM properly on all platforms

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
using namespace llvm;

static LLVMContext TheContext;


int main(int argc, char** argv) {

    Module* TheModule = new Module("my cool jit", TheContext);

    if (TheModule != nullptr)
        std::cout << "win" << std::endl;
    else
        std::cout << "faii" << std::endl;

    return 0;
}

#else

#include <iostream>
#include "include/tslversion.h"
#include "thirdparty/gtest/gtest.h"

int main(int argc, char** argv){
    std::cout << "--------------------------  " TSL_INTRO_STRING "  --------------------------" << std::endl;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif