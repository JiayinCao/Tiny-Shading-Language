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

// This file has a bunch of helper utility functions so that the rest of the program doesn't
// need to care about the details hiden here.

#pragma once

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "ast.h"

inline llvm::LLVMContext&   get_llvm_context(Tsl_Namespace::LLVM_Compile_Context& context) {
    return *context.context;
}

inline llvm::IRBuilder<>&   get_llvm_builder(Tsl_Namespace::LLVM_Compile_Context& context) {
    return *context.builder;
}

inline llvm::Module&        get_llvm_module(Tsl_Namespace::LLVM_Compile_Context& context) {
    return *context.module;
}

inline llvm::IntegerType*   get_int_1_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getInt1Ty(get_llvm_context(context));
}

inline llvm::IntegerType*   get_int_32_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getInt32Ty(get_llvm_context(context));
}

inline llvm::Type*          get_float_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getFloatTy(get_llvm_context(context));
}

inline llvm::Type*          get_float_ptr_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getFloatPtrTy(get_llvm_context(context));
}

inline llvm::Type*          get_double_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getDoubleTy(get_llvm_context(context));
}

inline llvm::Type*          get_void_ty(Tsl_Namespace::LLVM_Compile_Context& context) {
    return llvm::Type::getVoidTy(get_llvm_context(context));
}

inline llvm::Type*          get_type_from_context(const Tsl_Namespace::DataType type, Tsl_Namespace::LLVM_Compile_Context& context){
    switch (type) {
        case Tsl_Namespace::DataType::INT:
            return get_int_32_ty(context);
        case Tsl_Namespace::DataType::FLOAT:
            return get_float_ty(context);
        case Tsl_Namespace::DataType::FLOAT3:
            return get_float_ptr_ty(context);
        case Tsl_Namespace::DataType::FLOAT4:
            return get_float_ptr_ty(context);
        case Tsl_Namespace::DataType::DOUBLE:
            return get_double_ty(context);
        case Tsl_Namespace::DataType::MATRIX:
            return get_float_ptr_ty(context);
        case Tsl_Namespace::DataType::VOID:
            return get_void_ty(context);
        case Tsl_Namespace::DataType::BOOL:
            return get_int_1_ty(context);
        default:
            break;
    }
    return nullptr;
}

inline llvm::Type* get_type_from_context(const std::string& type, Tsl_Namespace::LLVM_Compile_Context& context) {
    if( type == "int" )
        return get_int_32_ty(context);
    else if( type == "float" )
        return get_float_ty(context);
    else if( type == "float3" )
        return get_float_ptr_ty(context);
    else if( type == "float4" )
        return get_float_ptr_ty(context);
    else if( type == "double" )
        return get_double_ty(context);
    else if( type == "matrix" )
        return get_float_ptr_ty(context);
    else if( type == "void" )
        return get_void_ty(context);
    else if( type == "bool" )
        return get_int_1_ty(context);
    return nullptr;
}

inline llvm::Value*     get_llvm_constant_fp(const float v, Tsl_Namespace::LLVM_Compile_Context& context){
    auto& llvm_context = get_llvm_context(context);
    return llvm::ConstantFP::get(llvm_context, llvm::APFloat(v));
}

inline llvm::Value*     get_llvm_constant_int(const int v, const int bw, Tsl_Namespace::LLVM_Compile_Context& context) {
    auto& llvm_context = get_llvm_context(context);
    return llvm::ConstantInt::get(llvm_context, llvm::APInt(bw, v));
}

inline llvm::Value*     convert_to_bool(llvm::Value* value, Tsl_Namespace::LLVM_Compile_Context& context) {
    auto& builder = get_llvm_builder(context);
    if (value->getType() == get_float_ty(context))
        value = builder.CreateFCmpONE(value, get_llvm_constant_fp(0.0f, context));
    else if (!value->getType()->isIntegerTy(1))
        value = builder.CreateICmpNE(value, get_llvm_constant_int(0, value->getType()->getIntegerBitWidth(), context));
    return value;
}

inline llvm::Value* get_llvm_add(llvm::Value* left, llvm::Value* right, Tsl_Namespace::LLVM_Compile_Context& context) {
    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFAdd(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateAdd(left, right);

    return nullptr;
}

inline llvm::Value* get_llvm_sub(llvm::Value* left, llvm::Value* right, Tsl_Namespace::LLVM_Compile_Context& context) {
    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFSub(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateSub(left, right);

    return nullptr;
}

inline llvm::Value* get_llvm_mul(llvm::Value* left, llvm::Value* right, Tsl_Namespace::LLVM_Compile_Context& context) {
    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFMul(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateMul(left, right);

    return nullptr;
}

inline llvm::Value* get_llvm_div(llvm::Value* left, llvm::Value* right, Tsl_Namespace::LLVM_Compile_Context& context) {
    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFDiv(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateSDiv(left, right);

    return nullptr;
}

inline llvm::Value* get_llvm_mod(llvm::Value* left, llvm::Value* right, Tsl_Namespace::LLVM_Compile_Context& context) {
    if (left->getType() == get_float_ty(context) && right->getType() == get_float_ty(context))
        return context.builder->CreateFRem(left, right);
    if (left->getType() == get_int_32_ty(context) && right->getType() == get_int_32_ty(context))
        return context.builder->CreateSRem(left, right);

    return nullptr;
}

inline bool is_llvm_integer(llvm::Value* value) {
    return value->getType()->isIntegerTy();
}
