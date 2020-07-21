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

#include <random>
#include <math.h>

#define PI 3.1415926f

// basic vector data structure. it is a vector of three for multiple purposes, like color, position, vector.
struct Vec {
    double x, y, z;
    Vec(double x_=0.0) { x = x_; y = x_; z = x_; }
    Vec(double x_, double y_, double z_) { x = x_; y = y_; z = z_; }
    Vec operator+(const Vec& b) const { return Vec(x + b.x, y + b.y, z + b.z); }
    Vec operator-(const Vec& b) const { return Vec(x - b.x, y - b.y, z - b.z); }
    Vec operator*(double b) const { return Vec(x * b, y * b, z * b); }
    Vec mult(const Vec& b) const { return Vec(x * b.x, y * b.y, z * b.z); }
    Vec& norm() { return *this = *this * (1 / sqrt(x * x + y * y + z * z)); }
    double dot(const Vec& b) const { return x * b.x + y * b.y + z * b.z; } // cross: 
    Vec operator%(Vec& b) { return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x); }
};

// data structure representing a ray
struct Ray { Vec o, d; Ray(Vec o_, Vec d_) : o(o_), d(d_) {} };

// materialtype, this needs to be refactored with TSL integrated later.
enum Refl_t { DIFF, SPEC, REFR };  // material types, used in radiance() 

// data structure representing a sphere
struct Sphere {
    double rad;       // radius 
    Vec p, e, c;      // position, emission, color 
    Refl_t refl;      // reflection type (DIFFuse, SPECular, REFRactive) 
	const bool fn;	  // whether normal is flipped
    Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_, bool fn) :
        rad(rad_), p(p_), e(e_), c(c_), refl(refl_), fn(fn) {}

    // ray sphere intersection
    double intersect(const Ray& r) const { // returns distance, 0 if nohit 
        Vec op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0) return 0; else det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
};

// generate a random number
inline float    random_number() {
    static std::random_device r;
    static std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(0, 65535);
    return (float)uniform_dist(e1) / 65535.f;
}