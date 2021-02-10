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

#include "tsl_define.h"
#include "system/impl.h"

TSL_NAMESPACE_BEGIN

#ifdef TSL_ON_WINDOWS
    #define DLLEXPORT __declspec(dllexport)
#else
    #define DLLEXPORT
#endif

using generic_ptr = int*;

extern "C" {
    // allocate memory inside shaders
    DLLEXPORT int* TSL_MALLOC(int size) {
        return (int*)allocate_memory(size);
    }

    // 2D texture sample
    DLLEXPORT void TSL_TEXTURE2D_SAMPLE(generic_ptr ptr, float3* color, float u, float v) {
        sample_2d((const void*)ptr, u, v, *color);
    }

    // 2D texture sample alpha
    DLLEXPORT void TSL_TEXTURE2D_SAMPLE_ALPHA(generic_ptr ptr, float* alpha, float u, float v) {
        sample_alpha_2d((const void*)ptr, u, v, *alpha);
    }
}

TSL_NAMESPACE_END
