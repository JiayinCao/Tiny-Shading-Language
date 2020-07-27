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

#include <any>
#include <string>
#include <vector>
#include "tsl_version.h"
#include "tsl_define.h"

TSL_NAMESPACE_BEGIN

// -----------------------------------------------------------------------------------------------------------
// TSL global variable declaration.
// -----------------------------------------------------------------------------------------------------------

// Shader execution commonly needs to communicate with its host program, renderers. In TSL, most of the
// comminucation is done through this TSL Global data structure. It is like the constant buffer concept in GPU
// shaders, execpt that each shader execution comes with one specific instance of tsl global, unlike GPU shader
// instances will share constant buffers across thousands even more executions.
//
// In order to inform the TSL shader about the memory layout of tsl global, it is necessary to define a data
// structure the following way,
//
//   DECLARE_TSLGLOBAL_BEGIN(TslGlobal)
//   DECLARE_TSLGLOBAL_VAR(Tsl_float3, view_direction)
//   DECLARE_TSLGLOBAL_VAR(Tsl_float3, uvw)
//   DECLARE_TSLGLOBAL_VAR(Tsl_float, custom_variable)
//   DECLARE_TSLGLOBAL_END()
//
// As we can see from the above definition, there is view direction, uvw and another custom variable. The way it
// is used is totally up to renderers. Also, the layout of the global variable is also customizable from renderers
// to renderers. A common case is for surface shader having one type of tsl global and volume shader having another
// one.
// To implement the data structure, it is necessary to define it in a cpp file this way
//
//   IMPLEMENT_TSLGLOBAL_BEGIN(TslGlobal)
//   IMPLEMENT_TSLGLOBAL_VAR(Tsl_float3, view_direction)
//   IMPLEMENT_TSLGLOBAL_VAR(Tsl_float3, uvw)
//   IMPLEMENT_TSLGLOBAL_VAR(Tsl_float,  custom_variable)
//   IMPLEMENT_TSLGLOBAL_END()
//
// Before each shader execution, it is up to renderers to make sure a proper data structure is passed in through
// the form of pointer so that TSL shader can have a chance to load proper data.
// For example loading view_direction data in the above defined function works like this in a shader,
//
//   shader function_name(out float3 var){
//      var = global_value<view_direction>;
//   }
//
// A few things derserve our attention
//   - Trying to load a variable not defined in TSL will result in compilation error.
//   - Shader group has to register with a TSL global type that is consitent with its shader units'. If there is
//     two shader units with two different TSL global layout, it will result TSL_Resolving_InconsistentTSLGlobalType
//     during shader group compilation.

//! @brief  Global variable description.
struct GlobalVar {
    std::string m_name;
    std::string m_type;
    GlobalVar(const std::string& name, const std::string& type) :
        m_name(name), m_type(type) {}
};

//! @brief  Global var list helps to track the memory layout defined in TSL global.
struct GlobalVarList {
    std::vector<GlobalVar>  m_var_list;         // a list keeping track of all global variables defined in TSL global.

    TSL_INTERFACE GlobalVarList();
    TSL_INTERFACE GlobalVarList(const std::vector<GlobalVar>& var_list);
    TSL_INTERFACE GlobalVarList(const GlobalVarList&);
};

#define DECLARE_TSLGLOBAL_BEGIN(T)          struct T {
#define DECLARE_TSLGLOBAL_VAR(VT,V)         VT V;
#define DECLARE_TSLGLOBAL_END()             static Tsl_Namespace::GlobalVarList m_var_list; static bool shader_unit_register( Tsl_Namespace::ShaderUnitTemplate* sut ) { return sut->register_tsl_global( m_var_list ); } };

#define IMPLEMENT_TSLGLOBAL_BEGIN(T)        Tsl_Namespace::GlobalVarList T::m_var_list( std::vector<Tsl_Namespace::GlobalVar>({
#define IMPLEMENT_TSLGLOBAL_VAR(VT,V)       { Tsl_Namespace::GlobalVar( #V, #VT ) },
#define IMPLEMENT_TSLGLOBAL_END()           }) );


// -----------------------------------------------------------------------------------------------------------
// TSL closure handle.
// -----------------------------------------------------------------------------------------------------------

// Closure is the concept that defered certain operations from TSL to renderers. Closures will be composed 
// into a tree form and this will commonly be the result of TSL shader execution. The closure tree is just 
// another form to represent a linearized weighted combination of different closures. This perfectly matches 
// the BSDF system in most open-source renderers. Basically, a bsdf is also a linear combination of weighted 
// bxdf. Renderers could simply anaylyse the closure tree to reconstruct the bsdf in its shading system. 
// This is just one example of how TSL could be used in renderers. Things like volumetric rendering, in which 
// there is a different system than bsdf, TSL could also be very helpful.

using ClosureID = int;
constexpr ClosureID INVALID_CLOSURE_ID = 0;
constexpr ClosureID CLOSURE_ADD = -1;
constexpr ClosureID CLOSURE_MUL = -2;

using ClosureParamPtr = void*;

//! @brief  Basic data structure maps to a closure in renderers.
/**
 * This is the data structure that points to a closure or another closure tree node. The id field identifies 
 * which closure it is. A negative id means the closure node could be an adding closure node ( it adds two 
 * closures together ) or a multiplication node ( it scales a closure ). A positive id will match to a 
 * specific closure to be explained in renderers.
 */
struct ClosureTreeNodeBase {
    ClosureID           m_id = INVALID_CLOSURE_ID;
    ClosureParamPtr     m_params = nullptr;
};

//! @brief  Closure adding together.
/**
 * Adding two closures together.
 */
struct ClosureTreeNodeAdd : public ClosureTreeNodeBase {
    ClosureTreeNodeBase* m_closure0 = nullptr;
    ClosureTreeNodeBase* m_closure1 = nullptr;
};

//! @brief  Scaling a closure.
/**
 * This is a closure node that scales a closure.
 */
struct ClosureTreeNodeMul : public ClosureTreeNodeBase {
    float m_weight = 1.0f;
    ClosureTreeNodeBase* m_closure = nullptr;
};

//! @brief  Closure tree is nothing but a pointer to the root node.
struct ClosureTree {
    ClosureTreeNodeBase* m_root = nullptr;
};

//! @brief  Closure argument.
/**
 * Each closure could have a few arguments when being constructed. Those argument could be constructed
 * from other shader instructions. This data structure keeps track of each arguments name and type.
 */
struct ClosureArg {
    const std::string m_name;       // name of the argument
    const std::string m_type;       // type of the argument
    ClosureArg(const std::string& name, const std::string& type) : m_name(name), m_type(type) {}
};
using ClosureArgList = std::vector<ClosureArg>;

// In order to claim a valid closure, it is necessary to define them in the way TSL expect. Though it is
// technically possible to register closure manually, it is strongly recommanded to go through these macros
// which already handles the lower level details.
//
// For example, claiming a lambert closure could goes like this in a header file,
//    DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert, "lambert")
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, base_color)
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, normal)
//    DECLARE_CLOSURE_TYPE_END(ClosureTypeLambert)
//
// It is clearly shown in the above code that lambert has two different parameters passed, one is the base
// color, the other is the normal to support normal map.
//
// Of course, apart from defining it, it is also necessary to find a cpp file to implement the structure this
// way,
//    IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
//    IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, base_color)
//    IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, Tsl_float3, normal)
//    IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeLambert)
//
// Internally, this will creates a list of ClosureArg so that the compiler knows about the detail of it.
//
// One important thing to mention here is that Closure itself could be used as an argument for constructing
// another closure type, like the commonly available bxdf in renderers like Coat, which coats another glossy
// layer on top of another brdf that could be any other brdf in the system. In this case, one can simply define
// the following code to implement it
//    DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeCoat, "coat")
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_closure, closure)
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float, roughness)
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float, ior)
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float3, sigma)
//    DECLARE_CLOSURE_TYPE_VAR(ClosureTypeCoat, Tsl_float3, normal)
//    DECLARE_CLOSURE_TYPE_END(ClosureTypeCoat)
//
// With the data structure declared in the source code, it is necessary to register it in the TSL shading system
// before compiling shader so that by the time the compiler sees the keyword, it is able to translate it to
// correct machine code.
//    ClosureTypeLambert::RegisterClosure();
//
// After the above configuration, it is fairly easy to construct a closure inside TSL shader, just do this,
//    shader closure_make(out closure o0){
//       color3 base_color = color3(1.0f, 1.0f, 1.0f);
//       vector normal = vector(0.0f, 1.0f, 0.0f);
//       o0 = make_closure<lambert>(base_color, normal);
//    }

#define DECLARE_CLOSURE_TYPE_BEGIN(T, name)     struct T { static const char* get_name() { return name; }
#define DECLARE_CLOSURE_TYPE_VAR(T,VT,V)        VT V;
#define DECLARE_CLOSURE_TYPE_END(T)             static Tsl_Namespace::ClosureArgList m_closure_args; static Tsl_Namespace::ClosureID RegisterClosure(); };

#define IMPLEMENT_CLOSURE_TYPE_BEGIN(T)         Tsl_Namespace::ClosureArgList T::m_closure_args({
#define IMPLEMENT_CLOSURE_TYPE_VAR(T,VT,V)      { Tsl_Namespace::ClosureArg( #V, #VT ) },
#define IMPLEMENT_CLOSURE_TYPE_END(T)           }); Tsl_Namespace::ClosureID T::RegisterClosure() { return Tsl_Namespace::ShadingSystem::get_instance().register_closure_type( T::get_name() , m_closure_args , sizeof(T) ); }

// It is very important to make sure the memory layout is as expected, there should be no fancy stuff compiler tries to do for these data structure.
// Because the same data structure will also be generated from LLVM, which will expect this exact memory layout. If there is miss-match, it will crash.
static_assert(sizeof(ClosureTreeNodeBase) == sizeof(ClosureID) + sizeof(ClosureParamPtr) + 4 /* memory padding. */, "Invalid Closure Tree Node Size");
static_assert(sizeof(ClosureTreeNodeAdd) == sizeof(ClosureTreeNodeBase) + sizeof(ClosureTreeNodeBase*) * 2, "Invalid ClosureTreeNodeAdd Node Size");
static_assert(sizeof(ClosureTreeNodeMul) == sizeof(ClosureTreeNodeBase) + sizeof(float) + 4 /* memory padding. */ + sizeof(ClosureTreeNodeBase*), "Invalid ClosureTreeNodeMul Node Size");


// -----------------------------------------------------------------------------------------------------------
// TSL function argument declaration.
// -----------------------------------------------------------------------------------------------------------

//! @brief  Basic float3 defined in TSL.
/**
 * It is intentionally to make it as simple as possible.
 * All related methods are inline and global to operate on it.
 */
struct float3 {
    float x, y, z;
};

/**
 * Some parameter could have a default value coming from the TSL global data structure.
 * This is the type of data structure that defines it.
 * The exact data will be fetched from TSL global during compilation, only a name is needed
 * here to indicate the memory address to locate it.
 */
struct ShaderUnitInputTslGlobalRef {
    std::string     m_name;
};

//! @brief  Default value for shader template argument.
/**
 * Sometimes exposed input value of a shader unit template defined in a shader group template
 * doesn't have anything connected. It is necessary to have this defined so that TSL knows what
 * value starts with.
 * Failing to define default value for unconnected shader unit template inputs will result
 * in shader group resolving failure.
 */
using ShaderUnitInputDefaultValue = std::any;

// helper function to make a data structure to reference tsl global
inline ShaderUnitInputTslGlobalRef make_tsl_global_ref(const std::string& name) {
    return { name };
}

// Following are just some helper function to make things easier
inline float3 make_float3(float x, float y, float z) {
    float3 ret; ret.x = x; ret.y = y; ret.z = z;; return ret;
}
inline float3 make_float3(float x) {
    return make_float3(x, x, x);
}
inline float3 operator + (const float3& a, const float3& b) {
    return make_float3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline float3 operator - (const float3& a, const float3& b) {
    return make_float3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline float3 operator * (const float3& a, const float3& b) {
    return make_float3(a.x * b.x, a.y * b.y, a.z * b.z);
}
inline float3 operator / (const float3& a, const float3& b) {
    return make_float3(a.x / b.x, a.y / b.y, a.z / b.z);
}
inline float3 operator + (const float3& a, const float b) {
    return make_float3(a.x + b, a.y + b, a.z + b);
}
inline float3 operator - (const float3& a, const float b) {
    return make_float3(a.x - b, a.y - b, a.z - b);
}
inline float3 operator * (const float3& a, const float b) {
    return make_float3(a.x * b, a.y * b, a.z * b);
}
inline float3 operator / (const float3& a, const float b) {
    return make_float3(a.x / b, a.y / b, a.z / b);
}
inline float3 operator + (const float a, const float3& b) {
    return make_float3(a + b.x, a + b.y, a + b.z);
}
inline float3 operator - (const float a, const float3& b) {
    return make_float3(a - b.x, a - b.y, a - b.z);
}
inline float3 operator * (const float a, const float3& b) {
    return make_float3(a * b.x, a * b.y, a * b.z);
}
inline float3 operator / (const float a, const float3& b) {
    return make_float3(a / b.x, a / b.y, a / b.z);
}
inline float dot(const float3& a, const float3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline float3 cross(const float3& a, const float3& b) {
    return make_float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Following are the only options for declaring a variable
#define Tsl_float3    Tsl_Namespace::float3
#define Tsl_float     float
#define Tsl_int       int
#define Tsl_bool      bool
#define Tsl_closure   void*
#define Tsl_double    double
#define Tsl_resource  void*

TSL_NAMESPACE_END