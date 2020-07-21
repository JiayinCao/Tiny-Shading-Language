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

#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>
#include "rt_material.h"
#include "rt_common.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// #define ENABLE_MULTI_THREAD_RAY_TRACING

#if 1
// scene description, all surfaces are spheres in this sample, including the wall, which are simply huge spheres.
Sphere spheres[] = {
  //Scene: radius, position,                    emission,       color,              material 
  Sphere(  1e5,    Vec(1e5 + 1,40.8,81.6),      Vec(),          Vec(.75,.25,.25),   DIFF),    //Left 
  Sphere(  1e5,    Vec(-1e5 + 99,40.8,81.6),    Vec(),          Vec(.25,.25,.75),   DIFF),    //Rght 
  Sphere(  1e5,    Vec(50, 40.8, 1e5),          Vec(),          Vec(.75,.75,.75),   DIFF),    //Back 
  Sphere(  1e5,    Vec(50, 40.8,-1e5 + 170),    Vec(),          Vec(),              DIFF),    //Frnt 
  Sphere(  1e5,    Vec(50, 1e5, 81.6),          Vec(),          Vec(.75,.75,.75),   DIFF),    //Botm 
  Sphere(  1e5,    Vec(50, 1e5 + 81.6,81.6),    Vec(),          Vec(.75,.75,.75),   DIFF),    //Top 
  Sphere(  16.5,   Vec(27, 16.5,47),            Vec(),          Vec(1,1,1) * .999,  SPEC),    //Mirr 
  Sphere(  16.5,   Vec(73, 20.5,78),            Vec(),          Vec(1,1,1) * .999,  REFR),    //Glas 
  Sphere(  600,    Vec(50, 681.6 - .27,81.6),   Vec(12,12,12),  Vec(),              DIFF)     //Lite 
};
#else
Sphere spheres[] = {//Scene: radius, position, emission, color, material 
  Sphere(1e5, Vec(50, 1e5, 81.6),       Vec(0.2,0,0.4),          Vec(.75,.75,.75),   DIFF),    //Botm 
  Sphere(600, Vec(50,681.6 - .27,81.6), Vec(12,12,12),  Vec(),              DIFF)               //Lite 
};
#endif

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

// the core of path tracing algorithm
Vec radiance(Ray r) {
#if 0
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
        double r1 = 2.f * PI * random_number(), r2 = random_number(), r2s = sqrt(r2);
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
#endif

    auto depth = 0;
    auto l = Vec();
    auto thr = Vec(1.f, 1.f, 1.f);
    while (true) {
        // get the intersection of the scene with the ray, there is no spatial acceleration data structure.
        // since there are only a few spheres in it, this is a brute-force O(N) algorithm.
        double t; int id = 0;
        if (!intersect(r, t, id))
            break;

        // the hit object
        const Sphere& obj = spheres[id];

        if (id == 5 && depth == 0)
            int k = 0;

        // accumulate the radiance
        l = l + thr.mult(obj.e);

        // the position of intersection
        Vec p = r.o + r.d * t;

        // all materials are lambert, this will be replaced with TSL driven 
        Lambert lambert(obj.c, obj.p);

        Vec wi;
        float pdf = 1.0f;
        const auto bxdf = lambert.sample(p, Vec(-r.d.x, -r.d.y, -r.d.z), wi, pdf);

        if (pdf <= 0.0f)
            break;

        thr = thr.mult(bxdf.mult(1.0f/pdf));

        // starting from the fifth recursive call, bail if possible
        ++depth;
        // this is to prevent stack overflow, it introduces a bit of bias, which is acceptable to me
        if (depth > 20)
            break;
        else if (depth > 5) {
            const auto p = 0.2f;
            if (random_number() < p)
                thr = thr * (1 / p);
            else
                break;
        }

        r.o = p + wi * 0.01f;
        r.d = wi;
    }
    return l;
}

int rt_main(int samps) {
    const auto w = 1024u, h = 768u;    // resolution of the image
    const auto inv_samps = 1.0f / samps;
    const auto total_pixel_cnt = w * h;

    // camera position and direction
    auto cam = Ray(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());
    auto cx = Vec(w * .5135 / h, 0.0f, 0.0f), cy = Vec((cx % cam.d).norm() * .5135);

    // for each pixel, take a number of samples and resolve them together.
    auto c = new Vec[total_pixel_cnt];// std::make_unique<Vec[]>(total_pixel_cnt);

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
                                r = r + radiance(Ray(cam.o + d * 140, d.norm())) * inv_samps;
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
    while (pixel_cnt < total_pixel_cnt) {
        int k = pixel_cnt;
        fprintf(stderr, "\rRendering (%d spp) %5.2f%% %d %d", samps * 4, 100. * pixel_cnt / total_pixel_cnt, k, total_pixel_cnt);
    }
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
                        r = r + radiance(Ray(cam.o + d * 140, d.norm())) * inv_samps;
                    }
                    c[i] = c[i] + r * 0.25f;
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