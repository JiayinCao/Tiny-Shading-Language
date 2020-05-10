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

// The purpose of these unit tests is to make sure the LLVM version that it is used to compile TSL supports the basic
// feature needed to implement TSL properly.

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

namespace {
	class LLVM : public testing::Test {
	public:
		LLVM(){
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();

			module = std::make_unique<llvm::Module>("my cool jit", context);
			EXPECT_NE(module, nullptr);
		}

		~LLVM() = default;

		ExecutionEngine* get_execution_engine(){
			if(nullptr == execute_engine)
				execute_engine = std::unique_ptr<ExecutionEngine>(llvm::EngineBuilder(std::move(module)).create());
			return execute_engine.get();
		}

		template<class T>
		T get_function(const char* function_name){
			auto ee = get_execution_engine();
			return (T) ee->getFunctionAddress(function_name);
		}

		llvm::LLVMContext context;
		std::unique_ptr<llvm::Module> module = nullptr;
		std::unique_ptr<llvm::ExecutionEngine> execute_engine = nullptr;
	};
}

/*
 * Manually composing a shader like this through LLVM without Flex and Bison
 *    int return_123(){
 *        return 123;
 *    }
 * And it should return 123 when JITed.
 */
TEST_F(LLVM, JIT) {
	Function *function = Function::Create(FunctionType::get(Type::getInt32Ty(context), {}, false), Function::ExternalLinkage, "return_123", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);
	builder.CreateRet(builder.getInt32(123));

	const auto gv = get_execution_engine()->runFunction(function, {});
	EXPECT_EQ(gv.IntVal, 123);
}

/*
 * Manually trying to call an external function defined in C++.
 *    float my_proxy_function(){
 *        float input_var = 12.0f;
 *        return external_cpp_function(input_var);
 *    }
 * And it should return what external_cpp_function returns when JITed.
 */

extern "C" DLLEXPORT float llvm_test_external_cpp_function( float x ) {
	return x * x;
}

TEST_F(LLVM, JIT_Ext_Func) {
	const float input_var = 12.0;

	// create external function prototype
	// this should perfectly match 'llvm_test_external_cpp_function' defined above.
	std::vector<Type *> proto_args(1, Type::getFloatTy(context));
	Function *ext_function = Function::Create(FunctionType::get(Type::getFloatTy(context), proto_args, false), Function::ExternalLinkage, "llvm_test_external_cpp_function", module.get());

	// the main function to be executed
	Function *function = Function::Create(FunctionType::get(Type::getFloatTy(context), {}, false), Function::ExternalLinkage, "my_proxy_function", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// call the external defined function in C++, llvm_test_external_cpp_function
	std::vector<Value *> args(1, ConstantFP::get(context, APFloat(input_var)));
	Value* value = builder.CreateCall(ext_function, args, "calltmp");

	// return whatever the call returns
	builder.CreateRet(value);

	// execute the jited function
	GenericValue gv = get_execution_engine()->runFunction(function, {});
	
	const float expected_result = llvm_test_external_cpp_function(input_var);
	EXPECT_EQ(gv.FloatVal, expected_result);
}

TEST_F(LLVM, Mutable_Variable) {
	const float input_var = 12.0;

	// create external function prototype
	// this should perfectly match 'llvm_test_external_cpp_function' defined above.
	std::vector<Type *> proto_args(1, Type::getFloatTy(context));
	Function *ext_function = Function::Create(FunctionType::get(Type::getFloatTy(context), proto_args, false), Function::ExternalLinkage, "llvm_test_external_cpp_function", module.get());

	// the main function to be executed
	Function *function = Function::Create(FunctionType::get(Type::getFloatTy(context), {}, false), Function::ExternalLinkage, "my_proxy_function", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// call the external defined function in C++, llvm_test_external_cpp_function
	std::vector<Value *> args(1);
	args[0] = ConstantFP::get(context, APFloat(input_var));
	Value* value = builder.CreateCall(ext_function, args, "calltmp");

	// return whatever the call returns
	builder.CreateRet(value);

	// execute the jited function
	std::vector<GenericValue> noargs;
	GenericValue gv = get_execution_engine()->runFunction(function, {});

	const float expected_result = llvm_test_external_cpp_function(input_var);
	EXPECT_EQ(gv.FloatVal, expected_result);
}

TEST_F(LLVM, JIT_Function_Pointer) {
	const float input_var = 12.0;

	// create external function prototype
	// this should perfectly match 'llvm_test_external_cpp_function' defined above.
	std::vector<Type *> proto_args(1, Type::getFloatTy(context));
	Function *ext_function = Function::Create(FunctionType::get(Type::getFloatTy(context), proto_args, false), Function::ExternalLinkage, "llvm_test_external_cpp_function", module.get());

	// the main function to be executed
	Function *function = Function::Create(FunctionType::get(Type::getFloatTy(context), {}, false), Function::ExternalLinkage, "my_proxy_function", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// call the external defined function in C++, llvm_test_external_cpp_function
	std::vector<Value *> args(1, ConstantFP::get(context, APFloat(input_var)));
	Value* value = builder.CreateCall(ext_function, args, "calltmp");

	// return whatever the call returns
	builder.CreateRet(value);

	// get the function pointer
 	const auto shader_func = get_function<float(*)(float)>("my_proxy_function");

	const float returned_value = shader_func(input_var);
	const float expected_result = llvm_test_external_cpp_function(input_var);
	EXPECT_EQ(returned_value, expected_result);
}

/*
 * Create a shader code that has a float pointer as an argument.
 *   void shader_func( float* out_var ){
 *       *out_var = 3.0f;
 *   }
 * Passing the address of a floating point data and expect the correct result.
 */
TEST_F(LLVM, Output_Arg) {
	const float constant_var = 12.0;

	// the main function to be executed
	std::vector<Type*> arg_types( 1 );
	arg_types[0] = Type::getFloatPtrTy(context);
	FunctionType* function_prototype = FunctionType::get(Type::getVoidTy(context), arg_types, false);

	Function *function = Function::Create(function_prototype, Function::ExternalLinkage, "shader_func", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// there should be exactly one argument
	EXPECT_EQ( function->arg_size() , 1 );

	auto var_ptr = function->args().begin();
	Value* value = ConstantFP::get(context, APFloat(constant_var));
	builder.CreateStore(value, var_ptr);

	builder.CreateRetVoid();

	// call the function compiled by llvm
	const auto shader_func = get_function<void (*)(float*)>("shader_func");
	float local_value = 0.0f;
	shader_func(&local_value);

	EXPECT_EQ(local_value, constant_var);
}

/*
 * This unit test passes a float pointer and expect an externally c++ defined function to write it.
 *   void shader_func( float* out_var ){
 *       external_func_cpp( out_var );
 *   }
 */

extern "C" DLLEXPORT void external_func_cpp(float* x) {
	*x = 12.0f;
}

TEST_F(LLVM, Passthrough_Pointer) {
	// create external function prototype
	// this should perfectly match 'external_func_cpp' defined above.
	std::vector<Type *> proto_args(1, Type::getFloatPtrTy(context));
	Function *ext_function = Function::Create(FunctionType::get(Type::getVoidTy(context), proto_args, false), Function::ExternalLinkage, "external_func_cpp", module.get());

	// the main function to be executed
	std::vector<Type*> arg_types( 1 );
	arg_types[0] = Type::getFloatPtrTy(context);
	FunctionType* function_prototype = FunctionType::get(Type::getVoidTy(context), arg_types, false);

	Function *function = Function::Create(function_prototype, Function::ExternalLinkage, "shader_func", module.get());
	BasicBlock *bb = BasicBlock::Create(context, "EntryBlock", function);
	IRBuilder<> builder(bb);

	// there should be exactly one argument
	EXPECT_EQ( function->arg_size() , 1 );

	// call the external defined function in C++, llvm_test_external_cpp_function
	std::vector<Value *> args(1);
	args[0] = function->args().begin();
	builder.CreateCall(ext_function, args, "calltmp");

	builder.CreateRetVoid();

	// call the function compiled by llvm
	const auto shader_func = get_function<void (*)(float*)>("shader_func");
	float local_value = 0.0f;
	shader_func(&local_value);

	EXPECT_EQ(local_value, 12.0f);
}