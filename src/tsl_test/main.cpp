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
    This project is mainly for the purpose of verifying features offered by TSL are properly executed
    as expected. It has quite a few number of unit tests that verify lots of areas in the shading
    language. If anything inside the shading language breaks, there is a good chance it will fail at
    least one of the unit tests in it.
    Every commit show make sure all unit tests are passed before checking in on Git. Otherwise, it means
    there is a bug in the system.
    
    WARNING, since it is a shading language, there is for sure tons of corner cases which I didn't cover
    that could also break the system. I will try covering as much as possible in the future, but since 
    it already replaced all features offered by OSL in my renderer SORT (http://sort-renderer.com/), the
    priority of populating this project with more unit tests is relatively low.
*/

#include <iostream>
#include "gtest/gtest.h"
#include "tsl_system.h"
#include "test/test_common.h"

// this is fairly ugly, but this is just unit test, I can live with it.
extern void* ptr;

class ShadingSystemInterfaceSimple : public Tsl_Namespace::ShadingSystemInterface {
public:
    // This is by no mean a good example of allocating memory of bxdf in real renderer.
    // The purpose of this code is simply for testing, not for performance.
    void* allocate(unsigned int size, void* tsl_global) const override {
        m_memory_holder.push_back(std::move(std::make_unique<char[]>(size)));
        return m_memory_holder.back().get();
    }

    // No error will be output since there are invalid unit tests.
    void catch_debug(const Tsl_Namespace::TSL_DEBUG_LEVEL level, const char* error) const override {
#if defined(TSL_DEBUG)
        std::cout << error << std::endl;
#endif
    }

    // Sample texture 2d
    void    sample_2d(const void* texture, float u, float v, float3& color) const override {
        auto ts = (const TextureSimple*)texture;
        color = ts->sample2d(u, v);
    }
    void    sample_alpha_2d(const void* texture, float u, float v, float& alpha) const override {
        auto ts = (const TextureSimple*)texture;
        alpha = ts->sample_alpha_2d(u, v);
    }

private:
    mutable std::vector<std::unique_ptr<char[]>> m_memory_holder;
};

ClosureID g_lambert_closure_id = INVALID_CLOSURE_ID;
ClosureID g_random_closure_id = INVALID_CLOSURE_ID;
ClosureID g_bxdf_with_double_id = INVALID_CLOSURE_ID;
ClosureID g_microfacete_id = INVALID_CLOSURE_ID;
ClosureID g_layered_bxdf_id = INVALID_CLOSURE_ID;
ClosureID g_lambert_in_sort_id = INVALID_CLOSURE_ID;
ClosureID g_measured_brdf_id = INVALID_CLOSURE_ID;

int main(int argc, char** argv) {
    std::cout << "--------------------------  " TSL_INTRO_STRING "  --------------------------" << std::endl;

    auto& shading_system = Tsl_Namespace::ShadingSystem::get_instance();

    std::unique_ptr<ShadingSystemInterfaceSimple> ssis = std::make_unique< ShadingSystemInterfaceSimple>();
    shading_system.register_shadingsystem_interface(std::move(ssis));

    // register all closure types
    g_lambert_closure_id = ClosureTypeLambert::RegisterClosure();
    g_random_closure_id  = ClosureTypeRandom0::RegisterClosure();
    g_bxdf_with_double_id = ClosureTypeBxdfWithDouble::RegisterClosure();
    g_microfacete_id = ClosureTypeMicrofacet::RegisterClosure();
    g_layered_bxdf_id = ClosureTypeLayeredBxdf::RegisterClosure();
    g_lambert_in_sort_id = ClosureTypeLambertInSORT::RegisterClosure();
    g_measured_brdf_id = ClosureTypeMeasuredBrdf::RegisterClosure();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}