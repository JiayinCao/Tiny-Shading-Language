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

#include <memory>
#include <tsl_version.h>
#include <tsl_args.h>
#include <tsl_system.h>
#include <tsl_define.h>
#include "rt_bxdf.h"

USE_TSL_NAMESPACE

// Initialize tiny shading language system
// It basically takes the chances to initialize all necessary data structure, like registering callback,
// closure type, tsl global data structure.
// This is considered the set up of TSL system, it is totally fine to leave it in one single thread, since
// it is fairly cheap to do so, multi-threading the initialization literally means nothing.
void initialize_tsl_system();

// reset memory pool counter
void reset_memory_allocator();

// Given a sphere object, acquire the material based on it.
// Note, the bxdf is returned through a smart pointer, this is by no means a performant way of doing this.
// In a reasonable complex ray tracing program, one may need to implement a memory pool to allocate bxdf
// in a much cheapper manner to avoid memory allocation overhead.
// However, this is not the focus of this program, I chose to live with it.
std::unique_ptr<Bxdf> get_bxdf(const Sphere& sphere);

// tsl global data structure
DECLARE_TSLGLOBAL_BEGIN(TslGlobal)
DECLARE_TSLGLOBAL_VAR(float3,   base_color)
DECLARE_TSLGLOBAL_VAR(float3,   center)
DECLARE_TSLGLOBAL_VAR(bool,     flip_normal)
DECLARE_TSLGLOBAL_END()

// closure for lambert type
DECLARE_CLOSURE_TYPE_BEGIN(ClosureTypeLambert, "lambert")
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, float3, base_color)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, float3, sphere_center)
DECLARE_CLOSURE_TYPE_VAR(ClosureTypeLambert, bool,   flip_normal)
DECLARE_CLOSURE_TYPE_END(ClosureTypeLambert)