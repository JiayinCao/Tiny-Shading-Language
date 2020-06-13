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

#include "shading_system.h"
#include "shading_context.h"
#include "shader_arg_types.h"

TSL_NAMESPACE_BEGIN

extern "C" {
    // allocate memory inside shaders
    int* TSL_ALLOC(int size, generic_ptr ptr) {
        auto allocator = (const MemoryAllocator*)ptr;
        return (int*)allocator->allocate(size);
    }

    // 2D texture access
    float4 TSL_TEXTURE_SAMPLE_2D(generic_ptr ptr, float u, float v) {
        // to be implemented
        return float4();
    }

    // 3D texture access
    float4 TSL_TEXTURE_SAMPLE_3D(generic_ptr ptr, float u, float v, float w) {
        // to be implemented
        return float4();
    }
}

TSL_NAMESPACE_END
