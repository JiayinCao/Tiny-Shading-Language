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

#include "test_common.h"

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLambert)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, int, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambert, float, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeLambert)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeMicrofacet)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMicrofacet, float, specular)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeMicrofacet)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLayeredBxdf)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, float, specular)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLayeredBxdf, void*, closure)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeLayeredBxdf)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeRandom0)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeRandom0, float3, roughness)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeRandom0)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeBxdfWithDouble)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, double, roughness)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeBxdfWithDouble, float, specular)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeBxdfWithDouble)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeLambertInSORT)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambertInSORT, float3, base_color)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeLambertInSORT, float3, normal)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeLambertInSORT)

IMPLEMENT_CLOSURE_TYPE_BEGIN(ClosureTypeMeasuredBrdf)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMeasuredBrdf, int, signature)
IMPLEMENT_CLOSURE_TYPE_VAR(ClosureTypeMeasuredBrdf, void*, custom_data)
IMPLEMENT_CLOSURE_TYPE_END(ClosureTypeMeasuredBrdf)

int g_name_counter = 0;