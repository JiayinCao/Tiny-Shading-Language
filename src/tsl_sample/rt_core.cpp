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
 * This is a modified verion of small pt (http://www.kevinbeason.com/smallpt/), a program
 * that was originally less than 100 lines and does a unbiased path tracing algorithm.
 *
 * The reason I picked this project as the beginning of the sample is to take advantage of
 * other's work to save me a bit of time. Since this project is simple enough for me to get 
 * started quickly. My main focus in this project is to demonstrate the usage of TSL, it
 * doesn't need to come with a sophisticated ray tracer. Since I don't care about simplicity
 * and this program serves more as a tutorial, clarity is more important than simplicity,
 * the program below is heavily modified by myself with more comment to make thing sclear.
 *
 * TSL related logic is not handled here though.
 */

#include <math.h>
#include <random>
#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define ENABLE_MULTI_THREAD_RAY_TRACING

// basic vector data structure. it is a vector of three for multiple purposes, like color, position, vector.
struct Vec {
    double x, y, z;
    Vec(double x_ = 0, double y_ = 0, double z_ = 0) { x = x_; y = y_; z = z_; }
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
    Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_) :
        rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}

    // ray sphere intersection
    double intersect(const Ray& r) const { // returns distance, 0 if nohit 
        Vec op = p - r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0 
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0) return 0; else det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
};

// scene description, all surfaces are spheres in this sample, including the wall, which are simply huge spheres.
Sphere spheres[] = {//Scene: radius, position, emission, color, material 
  Sphere(1e5, Vec(1e5 + 1,40.8,81.6),   Vec(),          Vec(.75,.25,.25),   DIFF),    //Left 
  Sphere(1e5, Vec(-1e5 + 99,40.8,81.6), Vec(),          Vec(.25,.25,.75),   DIFF),    //Rght 
  Sphere(1e5, Vec(50,40.8, 1e5),        Vec(),          Vec(.75,.75,.75),   DIFF),    //Back 
  Sphere(1e5, Vec(50,40.8,-1e5 + 170),  Vec(),          Vec(),              DIFF),    //Frnt 
  Sphere(1e5, Vec(50, 1e5, 81.6),       Vec(),          Vec(.75,.75,.75),   DIFF),    //Botm 
  Sphere(1e5, Vec(50,-1e5 + 81.6,81.6), Vec(),          Vec(.75,.75,.75),   DIFF),    //Top 
  Sphere(16.5,Vec(27,16.5,47),          Vec(),          Vec(1,1,1) * .999,  SPEC),    //Mirr 
  Sphere(16.5,Vec(73,16.5,78),          Vec(),          Vec(1,1,1) * .999,  REFR),    //Glas 
  Sphere(600, Vec(50,681.6 - .27,81.6), Vec(12,12,12),  Vec(),              DIFF)     //Lite 
};

// helper function to make thing easier.
inline double   clamp(double x) { 
    return x < 0 ? 0 : x > 1 ? 1 : x; 
}
inline int      toInt(double x) { 
    return int(pow(clamp(x), 1 / 2.2) * 255 + .5); 
}
inline bool     intersect(const Ray& r, double& t, int& id) {
    double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;) if ((d = spheres[i].intersect(r)) && d < t) { t = d; id = i; }
    return t < inf;
}
inline float    random_number() {
    static std::random_device r;
    static std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(0, 65535);
    return (float)uniform_dist(e1) / 65535.f;
}

// the core of path tracing algorithm
Vec radiance(const Ray& r, int depth) {
    // get the intersection of the scene with the ray, there is no spatial acceleration data structure.
    // since there are only a few spheres in it, this is a brute-force O(N) algorithm.
    double t; int id = 0;
    if (!intersect(r, t, id)) 
        return Vec();

    // the hit object
    const Sphere& obj = spheres[id];
    Vec x = r.o + r.d * t, n = (x - obj.p).norm(), nl = n.dot(r.d) < 0 ? n : n * -1, f = obj.c;
    double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl 

    // starting from the fifth recursive call, bail if possible
    ++depth;
    // this is to prevent stack overflow, it introduces a bit of bias, which is acceptable to me
    if (depth > 20)
        return obj.e;
    else if (depth > 5) {
        if (random_number() < p)
            f = f * (1 / p);
        else
            return obj.e;
    }

    // surface shading, this is the part needs to be integrated with TSL later.
    if (obj.refl == DIFF) {                  
        // Ideal DIFFUSE reflection 
        double r1 = 2 * 3.1415926f * random_number(), r2 = random_number(), r2s = sqrt(r2);
        Vec w = nl, u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(), v = w % u;
        Vec d = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).norm();
        return obj.e + f.mult(radiance(Ray(x, d), depth));
    }
    else if (obj.refl == SPEC) {
        // Ideal SPECULAR reflection 
        return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth));
    }

    // refracted material
    Ray reflRay(x, r.d - n * 2 * n.dot(r.d));       // Ideal dielectric REFRACTION 
    bool into = n.dot(nl) > 0;                      // Ray from outside going in? 
    double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(nl), cos2t;
    if ((cos2t = 1 - nnt * nnt * (1 - ddn * ddn)) < 0)    // Total internal reflection 
        return obj.e + f.mult(radiance(reflRay, depth));
    Vec tdir = (r.d * nnt - n * ((into ? 1 : -1) * (ddn * nnt + sqrt(cos2t)))).norm();
    double a = nt - nc, b = nt + nc, R0 = a * a / (b * b), c = 1 - (into ? -ddn : tdir.dot(n));
    double Re = R0 + (1 - R0) * c * c * c * c * c, Tr = 1 - Re, P = .25 + .5 * Re, RP = Re / P, TP = Tr / (1 - P);
    return obj.e + f.mult(depth > 2 ? (random_number() < P ?   // Russian roulette 
        radiance(reflRay, depth) * RP : radiance(Ray(x, tdir), depth) * TP) :
        radiance(reflRay, depth) * Re + radiance(Ray(x, tdir), depth) * Tr);
}

int rt_main() {
    const auto w = 1024u, h = 768u;    // resolution of the image
    const auto samps = 4;            // sample per pixel
    const auto inv_samps = 1.0f / samps;
    const auto total_pixel_cnt = w * h;

    // camera position and direction
    auto cam = Ray(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());
    auto cx = Vec(w * .5135 / h), cy = Vec((cx % cam.d).norm() * .5135);

    // for each pixel, take a number of samples and resolve them together.
    auto c = std::make_unique<Vec[]>(total_pixel_cnt);

#ifdef ENABLE_MULTI_THREAD_RAY_TRACING
    // the number of physical cores available in CPU
    const auto processor_count = std::thread::hardware_concurrency();

    // each thread is responsible for a bunch of lines depending on how many physical cores are available on the platform.
    const auto batch_line_number = (h + processor_count - 1) / processor_count;
    const auto batch_number = h / batch_line_number;

    // thread pool
    std::vector<std::thread> threads(batch_number);

    // pixel counter
    std::atomic<int> pixel_cnt = 0;
    for (auto b = 0; b < batch_number; ++b) {
        const auto batch_offset = b * batch_line_number;
        const auto batch_max = std::min(batch_offset + batch_line_number, h);

        // spawn a seperate thread for ray tracing
        threads[b] = std::thread([&](unsigned batch_offset, unsigned batch_max) {
            for (auto y = batch_offset; y < batch_max; ++y) {
                for (unsigned short x = 0; x < w; x++) {
                    for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++) {
                        for (int sx = 0; sx < 2; sx++) {
                            auto r = Vec();
                            for (int s = 0; s < samps; s++) {
                                double r1 = 2.f * random_number(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                                double r2 = 2.f * random_number(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                                Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                                    cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                                r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0) * inv_samps;
                            }
                            c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z)) * 0.25f;
                        }
                    }
                    ++pixel_cnt;
                }
            }
        }, batch_offset, batch_max);
    }

    // this kind of sychronization is by no means the most performant one, but it does do its job
    while (pixel_cnt < total_pixel_cnt)
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * pixel_cnt / total_pixel_cnt);
#else
    for (int y = 0; y < h; y++) {
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100. * y / (h - 1));
        for (unsigned short x = 0; x < w; x++) {
            for (int sy = 0, i = (h - y - 1) * w + x; sy < 2; sy++) {
                for (int sx = 0; sx < 2; sx++) {
                    auto r = Vec();
                    for (int s = 0; s < samps; s++) {
                        double r1 = 2 * random_number(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * random_number(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                            cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                        r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0) * inv_samps;
                    }
                    c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z)) * 0.25f;
                }
            }
        }
    }
#endif
    
    // convert the floating point format to unsigned short int for stb to output the image
    auto ret = std::make_unique<char[]>(w * h * 3);
    for (int i = 0; i < w * h; i++) {
        auto k = i * 3;
        ret[k++] = toInt(c[i].x);
        ret[k++] = toInt(c[i].y);
        ret[k] = toInt(c[i].z);
    }
    stbi_write_jpg("tsl_sample.jpg", w, h, 3, ret.get(), 100);

    return 0;
}