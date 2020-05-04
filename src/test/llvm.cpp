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

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "thirdparty/gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
using namespace llvm;

// this test is purely just to verify llvm is properly setup.
TEST(LLVM, LLVM_Configuration) {
    llvm::LLVMContext TheContext;
	std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
    EXPECT_NE(TheModule.get(), nullptr);
}

TEST(LLVM, LLVM_JIT) {
	llvm::InitializeNativeTarget();
	llvm::InitializeNativeTargetAsmPrinter();

	llvm::LLVMContext TheContext;
	std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("my cool jit", TheContext);
	EXPECT_NE(TheModule, nullptr);

	/*
		create a simple function
	    int return_123(){
	       return 123;
	    }
	*/
	Function *function = Function::Create(FunctionType::get(Type::getInt32Ty(TheContext), {}, false), Function::ExternalLinkage, "return_123", TheModule.get());
	BasicBlock *bb = BasicBlock::Create(TheContext, "EntryBlock", function);
	IRBuilder<> builder(bb);
	builder.CreateRet(builder.getInt32(123));

	// function->print(llvm::errs());

	llvm::ExecutionEngine* EE = llvm::EngineBuilder(std::move(TheModule)).create();

	std::vector<GenericValue> noargs;
	GenericValue gv = EE->runFunction(function, noargs);
	
	EXPECT_EQ(gv.IntVal, 123);

	delete EE;
	llvm_shutdown();
}