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
 * This file demonstrate all it needs to integrate TSL in this ray tracer.
 */

#include <assert.h>
#include <memory>
#include "rt_tsl.h"

IMPLEMENT_TSLGLOBAL_BEGIN(TslGlobal)
IMPLEMENT_TSLGLOBAL_VAR(float3, base_color)
IMPLEMENT_TSLGLOBAL_VAR(float3, center)
IMPLEMENT_TSLGLOBAL_VAR(float,  radius)
IMPLEMENT_TSLGLOBAL_VAR(float3, position)
IMPLEMENT_TSLGLOBAL_VAR(bool,   flip_normal)
IMPLEMENT_TSLGLOBAL_END()

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, float3, sphere_center)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, bool, flip_normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeLambert)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacet)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float3, sphere_center)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, bool, flip_normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeMicrofacet)

// In an ideal world, a sophisticated renderer should have its own memory management system.
// For example, it could pre-allocate a memory pool and claim memory dynamically during bxdf 
// allocation. This way it can avoid the performance overhead of page allocation under the hood.
// In order to stay as simple as possible, the following code does demonstrate a similar idea.
// However, the big limitation is its memory size, once memory runs out, it will crash.
// This is fine for this simple program since it has a hard limit on the depth of recursive rays
// to traverse, meaning there is also a limitation of how much memory it will allocate.

// This is just a random big number that avoids memory running out.
constexpr int BUF_MEM_SIZE = 16866;
// The current buffer offset, needs to be reset before at the beginning of evaluating every single pixel.
static thread_local int  buf_index = 0;
// The pre-allocated buffer.
static thread_local char buf[BUF_MEM_SIZE];

// This is the call back function for handling things like compiling errors and texture loading stuff.
class ShadingSystemInterfaceSimple : public Tsl_Namespace::ShadingSystemInterface {
public:
    // Simply fetch some memory from the memory pool
    void* allocate(unsigned int size) const override {
        assert(buf_index + size < BUF_MEM_SIZE);
        void* ret = buf + buf_index;
        buf_index += size;
        return ret;
    }

    // No error will be output since there are invalid unit tests.
    void catch_debug(const Tsl_Namespace::TSL_DEBUG_LEVEL level, const char* error) const override {
        printf("%s\n", error);
    }

    // Sample texture 2d
    void    sample_2d(const void* texture, float u, float v, Tsl_Namespace::float3& color) const override {
        // not implemented
    }

    void    sample_alpha_2d(const void* texture, float u, float v, float& alpha) const override {
        // not implemented
    }
};

// The raw function pointer of all surface shaders.
using shader_raw_func = void(*)(Tsl_Namespace::ClosureTreeNodeBase**, TslGlobal*);

// This is a very thin layer to wrap TSL related data structures, in a real complex ray tracing algorithm,
// there could be way more members in it. But in this tutorial program, this is good enough, it has everything
// it needs to express the properties of the material.
struct Material {
    // This is the shader unit template
    std::shared_ptr<ShaderUnitTemplate> m_shader_template;

    // This is the resolved shader instance, this is the unit of shader execution.
    std::shared_ptr<ShaderInstance>     m_shader_instance;

    // the resolved raw function pointer
    shader_raw_func                     m_shader_func = nullptr;
};

// all materials available to use in this program
static Material g_materials[MaterialType::Cnt];

// Although it is possible to have different tsl global registered for different material types, this sample only uses one.
static TslGlobal g_tsl_global;

// The closure ids
static ClosureID g_closure_lambert      = INVALID_CLOSURE_ID;
static ClosureID g_closure_microfacet   = INVALID_CLOSURE_ID;

/*
 * The first material, lambert is very simple and straightforward. All of it is driven by one single shader unit template.
 * It is no the simplest form of TSL shader execution. Typically, renderers need to do things more complex than this material
 * since shaders are usually groupped by multiple shader unit templates.
 */
bool initialize_lambert_material() {
    constexpr auto shader_source = R"(
        // This is simply a passing through shader that pass the data from TSL to the closure lambert.
        shader lambert_shader(out closure bxdf){
            color  base_color   = global_value<base_color>;
            vector center       = global_value<center>;
            bool   flip_normal  = global_value<flip_normal>;

            // make a lambertian closure
            bxdf = make_closure<lambert>(base_color, center, flip_normal);
        }
    )";

    // Get the instance of TSL system
    auto& shading_system = Tsl_Namespace::ShadingSystem::get_instance();

    // Make a new shading context, instead of making a new context, renders can also cache a few shading context at the beginning.
    // And reuse them any time they are needed as long as no two threads are accessing the same shading context at the same time.
    auto shading_context = shading_system.make_shading_context();
    if (!shading_context)
        return false;

    // Get the first material, which is supposed to be lambert.
    auto& mat = g_materials[(int)MaterialType::MT_Lambert];

    // Create the shader unit template
    mat.m_shader_template = shading_context->begin_shader_unit_template("lambert");
    if (!mat.m_shader_template)
        return false;

    // Register the TSL global for this shader unit template.
    auto ret = mat.m_shader_template->register_tsl_global(g_tsl_global.m_var_list);
    if (!ret)
        return false;

    // Compile the shader source code.
    ret = mat.m_shader_template->compile_shader_source(shader_source);
    if (!ret)
        return false;

    // Indicating the end of the shader unit template creation process.
    auto resolved_ret = shading_context->end_shader_unit_template(mat.m_shader_template.get());
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;

    // make a shader instance
    mat.m_shader_instance = mat.m_shader_template->make_shader_instance();
    if (!mat.m_shader_instance)
        return false;

    // Resolve the shader instance
    resolved_ret = mat.m_shader_instance->resolve_shader_instance();
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;

    // get the raw function pointer
    mat.m_shader_func = (shader_raw_func)mat.m_shader_instance->get_function();

    return true;
}

/*
 * In this material, there is something more complex done through TSL. Instead of creating a single shader unit template,
 * there will be two of the shader unit templates connected together forming a shader group template.
 * It goes like this,
 *
 *   --------------------------------  Shader Group  -------------------------------------
 *   |                                                                                   |
 *   |  ------ Base Color Shader ------                ------ Microfacet Shader ------   |
 *   |  |                             |                |                             |   |
 *   |  |                         color -------------->base_color              closure   |
 *   |  |                             |                |                             |   |
 *   |  -------------------------------                -------------------------------   |
 *   |                                                                                   |
 *   -------------------------------------------------------------------------------------
 *
 * Instead of having constant properties for the whole material, this material takes advantages of the flexibility offered
 * by TSL and drives the roughness value based on its position. The higher the point is, the smoother it is. On the contrary,
 * the lower it is, the rougher it is.
 */
bool initialize_microfacet_material() {
    constexpr auto microfacet_shader_src = R"(
        float saturate( float x ){
            return ( x > 1.0f ) ? x : ( ( x < 0.0f ) ? 0.0f : x );
        }

        shader microfacet_shader(in color base_color, out closure bxdf){
            vector center       = global_value<center>;
            bool   flip_normal  = global_value<flip_normal>;
            
            // roughness is driven by position, the higher the point is, the smoother it is.
            vector position     = global_value<position>;
            float  radius       = global_value<radius>;
            float  roughness    = 1.0f - ( position.y - center.y + radius ) / ( 2.0f * radius );

            // make a microfacet closure
            bxdf = make_closure<microfacet>(base_color, saturate(roughness- 0.2f), center, flip_normal);
        }
    )";

    constexpr auto basecolor_shader_src = R"(
        // https://docs.unrealengine.com/en-US/Engine/Rendering/Materials/PhysicallyBased/index.html
        shader basecolor_shader(out color basecolor){
            basecolor = color(1.000f, 0.766f, 0.336f);
        }
    )";

    // Get the instance of TSL system
    auto& shading_system = Tsl_Namespace::ShadingSystem::get_instance();

    // Make a new shading context, instead of making a new context, renders can also cache a few shading context at the beginning.
    // And reuse them any time they are needed as long as no two threads are accessing the same shading context at the same time.
    auto shading_context = shading_system.make_shading_context();
    if (!shading_context)
        return false;

    // a microfacet driven material
    auto& mat = g_materials[(int)MaterialType::MT_Microfacet];

    // compile the microfacet shader unit template
    auto microfacet_shader = shading_context->begin_shader_group_template("microfacet_shader");
    auto ret = microfacet_shader->register_tsl_global(g_tsl_global.m_var_list);
    if(!ret)
        return false;
    ret = microfacet_shader->compile_shader_source(microfacet_shader_src);
    if(!ret)
        return false;
    auto resolved_ret = shading_context->end_shader_unit_template(microfacet_shader.get());
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;

    // compile the baecolor shader unit template
    auto basecolor_shader = shading_context->begin_shader_group_template("basecolor_shader");
    ret = basecolor_shader->register_tsl_global(g_tsl_global.m_var_list);
    if (!ret)
        return false;
    ret = basecolor_shader->compile_shader_source(basecolor_shader_src);
    if (!ret)
        return false;
    resolved_ret = shading_context->end_shader_unit_template(basecolor_shader.get());
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;
    

    // Create the shader group template
    auto shader_group = shading_context->begin_shader_group_template("microfacet shader group");
    if (!shader_group)
        return false;

    // Register the TSL global for this shader unit template.
    ret = shader_group->register_tsl_global(g_tsl_global.m_var_list);
    if (!ret)
        return false;

    // Add the two shaders
    shader_group->add_shader_unit("microfacet", microfacet_shader, true);
    shader_group->add_shader_unit("basecolor", basecolor_shader);

    // Setup the connection between the two shaders
    shader_group->connect_shader_units("basecolor", "basecolor", "microfacet", "base_color");

    // Expose the shader argument so that this argument can be accessed from host program
    shader_group->expose_shader_argument("microfacet", "bxdf");

    // Indicating the end of the shader unit template creation process.
    resolved_ret = shading_context->end_shader_group_template(shader_group.get());
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;

    // Forward the ownership now
    mat.m_shader_template = std::move(shader_group);

    // Make a shader instance
    mat.m_shader_instance = mat.m_shader_template->make_shader_instance();
    if (!mat.m_shader_instance)
        return false;

    // Resolve the shader instance
    resolved_ret = mat.m_shader_instance->resolve_shader_instance();
    if (resolved_ret != TSL_Resolving_Status::TSL_Resolving_Succeed)
        return false;

    // Get the raw function pointer
    mat.m_shader_func = (shader_raw_func)mat.m_shader_instance->get_function();

    return true;
}

// Initialize all materials
void initialize_materials() {
    initialize_lambert_material();
    initialize_microfacet_material();
}

// Reset the memory pool, this is a pretty cheap operation.
void reset_memory_allocator() {
    buf_index = 0;
}

/*
 * It does several things during TSL initialization.
 *   - Register the call back interface so that the ray tracer can handle some call back events like bxdf allocation.
 *   - Register all closure types used in this program. This needs to happen before shader compliation.
 *   - Create all materials by compiling its shader and cache the raw function pointer to be used later.
 */
void initialize_tsl_system() {
    // get the instance of tsl shading system
    auto& shading_system = Tsl_Namespace::ShadingSystem::get_instance();

    // register the call back functions
    std::unique_ptr<ShadingSystemInterfaceSimple> ssis = std::make_unique< ShadingSystemInterfaceSimple>();
    shading_system.register_shadingsystem_interface(std::move(ssis));

    // register closures
    g_closure_lambert       = ClosureTypeLambert::RegisterClosure();
    g_closure_microfacet    = ClosureTypeMicrofacet::RegisterClosure();

    // initialize all materials
    initialize_materials();
}

/*
 * Get the bxdf based on the sphere object. It basically gets the material based on the material type. With the material
 * located, it can easily access its resolved raw shader function with its compiled shader. It will then execute the shader
 * and parse the returned result from TSL shader to populate the data structure to be returned.
 */
std::unique_ptr<Bxdf> get_bxdf(const Sphere& obj, const Vec& p) {
    // setup tsl global data structure
    TslGlobal tsl_global;
    tsl_global.base_color = make_float3(obj.c.x, obj.c.y, obj.c.z);
    tsl_global.center = make_float3( obj.p.x , obj.p.y, obj.p.z );
    tsl_global.flip_normal = obj.fn;
    tsl_global.position = make_float3(p.x, p.y, p.z);
    tsl_global.radius = obj.rad;

    // get the material
    const auto& mat = g_materials[obj.mt];
    if (!mat.m_shader_func)
        return std::make_unique<Lambert>(Vec(1.0, 0.0, 0.0), obj.p, obj.fn);

    // execute tsl shader
    Tsl_Namespace::ClosureTreeNodeBase* closure = nullptr;
    mat.m_shader_func(&closure, &tsl_global);

    // parse the result
    if (closure->m_id == g_closure_lambert) {
        const ClosureTypeLambert* bxdf_param = (const ClosureTypeLambert*)closure->m_params;
        return std::make_unique<Lambert>(obj.c, bxdf_param->sphere_center, bxdf_param->flip_normal);
    }
    else if (closure->m_id == g_closure_microfacet) {
        const ClosureTypeMicrofacet* bxdf_param = (const ClosureTypeMicrofacet*)closure->m_params;
        return std::make_unique<Microfacet>(bxdf_param->base_color, bxdf_param->roughness, bxdf_param->sphere_center, bxdf_param->flip_normal);
    }

    // unrecognized closure type, it shouldn't reach here at all
    return std::make_unique<Lambert>(Vec(1.0, 0.0, 0.0), obj.p, obj.fn);
}