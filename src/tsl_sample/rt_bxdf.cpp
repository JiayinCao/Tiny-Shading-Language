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
#include "rt_common.h"
#include "rt_bxdf.h"

// uniformly sample a point on disk
void UniformSampleDisk(float& x, float& y) {
    const auto u = random_number(), v = random_number();
    const auto theta = 2.0f * PI * u;
    const auto radius = sqrt(v);
    x = radius * cos(theta);
    y = radius * sin(theta);
}

float clamp(const float x, const float a, const float b) {
    return (x < a) ? a : ((x > b) ? b : x);
}

Vec spherical_vec(float theta, float phi) {
    const auto x = sin(theta) * cos(phi);
    const auto y = cos(theta);
    const auto z = sin(theta) * sin(phi);
    return Vec(x, y, z);
}

float cos_theta(const Vec& v) {
    return v.y;
}

float cos_theta2(const Vec& v) {
    return SQR(cos_theta(v));
}

float tan_theta2(const Vec& w) {
    return 1.0f / cos_theta2(w) - 1.0f;
}

Vec cross(const Vec& v0, const Vec& v1) {
    return Vec( v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x);
}

float sin_theta2(const Vec& w) {
    return std::max(0.f, 1.f - cos_theta2(w));
}

float sin_theta(const Vec& w) {
    return sqrtf(sin_theta2(w));
}

float cos_phi(const Vec& w) {
    float sintheta = sin_theta(w);
    if (sintheta == 0.f) return 1.f;
    return clamp(w.x / sintheta, -1.f, 1.f);
}

float cos_phi2(const Vec& w) {
    return SQR(cos_phi(w));
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
    auto n = (pos - sphere_center).norm();

    if (flip_normal)
        n = n * -1.0f;

    Vec t, bt;
    coordinateSystem(n, t, bt);

    Vec ret;
    ret.x = t.x * vec.x + n.x * vec.y + bt.x * vec.z;
    ret.y = t.y * vec.x + n.y * vec.y + bt.y * vec.z;
    ret.z = t.z * vec.x + n.z * vec.y + bt.z * vec.z;
    return ret;
}

Vec Bxdf::world_to_local(const Vec& pos, const Vec& vec) {
    auto n = (pos - sphere_center).norm();

    if (flip_normal)
        n = n * -1.0f;

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

    if ( local_wo.y <= 0.0f ) {
        pdf = 0.0f;
        return Vec();
    }

    float x, z;
    UniformSampleDisk(x, z);
    float y = sqrt(std::max(0.0f, 1.0f - x * x - z * z));
    const auto local_wi = Vec(x, y, z);
    wi = local_to_world(pos, local_wi);
    
    pdf = fabs(local_wi.y) / PI;

    return basecolor * (fabs(local_wi.y) / PI) ;
}

Vec Microfacet::sample(const Vec& pos, const Vec& wo, Vec& wi, float& pdf) {
    const auto local_wo = world_to_local(pos, wo);
    if (local_wo.y <= 0.0f) {
        pdf = 0.0f;
        return Vec();
    }

    // importance sampling of GGX distribution
    // https://agraphicsguy.wordpress.com/2015/11/01/sampling-microfacet-brdf/
    const float u = random_number(), v = random_number();
    const auto theta = atan(alpha * std::sqrt(v / (1.0f - v)));
    const auto phi = TWO_PI * u;
    const auto h = spherical_vec(theta, phi);

    // reflect the wo along the sampled normal
    const auto local_wi = h * ( 2.0f * local_wo.dot(h) ) - local_wo;
    wi = local_to_world(pos, local_wi);

    // kill all sampled direction that is below the surface.
    auto n = (pos - sphere_center).norm();
    if (wi.dot(n) <= 0.0f) {
        pdf = 0.0f;
        return Vec();
    }

    // https://www.pbrt.org/
    // Anisotropic GGX (Trowbridge-Reitz) distribution formula, pbrt-v3 ( page 539 )
    const auto cos_theta_h_sq = cos_theta2(h);
    if (cos_theta_h_sq <= 0.0f) return 0.f;
    const auto ggx = 1.0f / (cos_theta_h_sq + (1.0f - cos_theta_h_sq) / alpha2);
    const auto nov = cos_theta(local_wo);

    // evaluate the pdf
    pdf = ggx * cos_theta(h) / (4.0f * local_wo.dot(h));

    // fresnel, to be done
    const auto fresnel = basecolor + ((Vec(1.0f) - basecolor) * pow(1.0f - nov, 5.0f));

    // visibility term
    // Smith shadow - masking function
    const auto smith_vis = [](const Vec& v, const float alpha2) {
        const auto tan_theta_sq = tan_theta2(v);
        if (isinf(tan_theta_sq)) return 0.0f;
        const auto cos_phi_sq = cos_phi2(v);
        return 2.0f / (1.0f + sqrt(1.0f + alpha2 * tan_theta_sq));
    };
    const auto vis = smith_vis(local_wo, alpha2) * smith_vis(local_wi, alpha2);

    // the final evaluated microfacet brdf multiplied by cos( n , wi )
    return fresnel * ( ggx * vis / (4.0f * nov));
}