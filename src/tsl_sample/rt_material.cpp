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

#include <algorithm>
#include "rt_material.h"

// uniformly sample a point on disk
void UniformSampleDisk(float u, float v, float& x, float& y) {
    float r, theta;
    float su = 2.0f * u - 1.0f;
    float sv = 2.0f * v - 1.0f;
    float asu = fabs(su);
    float asv = fabs(sv);

    if (asu < asv) {
        r = asv;
        float factor = (sv > 0.0f) ? -1.0f : 1.0f;
        theta = factor * su / r + 4.0f + 2.0f * factor;
    }
    else if (asv < asu) {
        r = asu;
        float factor = (su > 0.0f) ? 1.0f : -1.0f;
        theta = factor * sv / r - 2.0f * factor + 2.0f;
        if (theta < 0.0f)
            theta += 8.0f;
    }
    else {
        x = 0.0f;
        y = 0.0f;
        return;
    }

    theta *= PI / 4.0f;

    x = cos(theta) * r;
    y = sin(theta) * r;
}

Vec cross(const Vec& v0, const Vec& v1) {
    return Vec( v0.y * v1.z - v0.z * v1.y,
                v0.z * v1.x - v0.x * v1.z,
                v0.x * v1.y - v0.y * v1.x);
}

void coordinateSystem(const Vec& v0, Vec& v1, Vec& v2) {
    if (fabs(v0.x) > fabs(v0.y)) {
        float invLen = 1.0f / sqrtf(v0.x * v0.x + v0.z * v0.z);
        v1 = Vec(-v0.z * invLen, 0.0f, v0.x * invLen);
    }
    else {
        float invLen = 1.0f / sqrtf(v0.y * v0.y + v0.z * v0.z);
        v1 = Vec(0.0f, v0.z * invLen, -v0.y * invLen);
    }
    v2 = cross(v0, v1);
}

Vec Bxdf::local_to_world(const Vec& pos, const Vec& vec) {
    const Vec n = (pos - sphere_center).norm();
    Vec t, bt;
    coordinateSystem(n, t, bt);

    Vec ret;
    ret.x = t.x * vec.x + n.x * vec.y + bt.x * vec.z;
    ret.y = t.y * vec.x + n.y * vec.y + bt.y * vec.z;
    ret.z = t.z * vec.x + n.z * vec.y + bt.z * vec.z;
    return ret;
}

Vec Bxdf::world_to_local(const Vec& pos, const Vec& vec) {
    const Vec n = (pos - sphere_center).norm();

    if (n.y <= -1.0f + 0.001f)
        int k = 0;

    Vec t, bt;
    coordinateSystem(n, t, bt);

    Vec ret;
    ret.x = t.x * vec.x + t.y * vec.y + t.z * vec.z;
    ret.y = n.x * vec.x + n.y * vec.y + n.z * vec.z;
    ret.z = bt.x * vec.x + bt.y * vec.y + bt.z * vec.z;
    return ret;
}

Vec Lambert::sample(const Vec& pos, const Vec& wo, Vec& wi, float& pdf) {
    const auto local_wo = world_to_local(pos, wo);

    if ( false && local_wo.y >= 0.0f ) 
    {
        pdf = 0.0f;
        return Vec();
    }

    float x, z, u = random_number(), v = random_number();
    UniformSampleDisk(u, v, x, z);
    float y = sqrt(std::max(0.0f, 1.0f - x * x - z * z));
    const auto local_wi = Vec(x, y, z);
    wi = local_to_world(pos, local_wi);
    
    pdf = fabs(local_wi.y);

    return basecolor * (pdf / PI) ;
}