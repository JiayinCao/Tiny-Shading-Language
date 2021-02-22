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
 * This is a modified version of small pt (http://www.kevinbeason.com/smallpt/), a program
 * that was originally less than 100 lines and does a unbiased path tracing algorithm.
 *
 * The reason I picked this project as the beginning of the sample is to take advantage of
 * other's work to save me a bit of time. Since this project is simple enough for me to get 
 * started quickly. My main focus in this project is to demonstrate the usage of TSL, it
 * doesn't need to come with a sophisticated ray tracer. Since I don't care about simplicity
 * and this program serves more as a tutorial, clarity is more important than simplicity,
 * the program below is heavily modified by myself with more comment to make things clear.
 *
 * Note, although this program demonstrates how to integrate TSL in a ray tracer. The ray
 * tracing algorithm itself is by no means a perfect one. Quite a few things are done this
 * way simply because it is easy and straightforward to get it done this way. For example,
 * there is no multiple importance sampling, random samples taken on a disk is not uniformly
 * distributed, memory management is no where near its best efficiency, etc.
 */

#include <memory>
#include <thread>
#include <algorithm>
#include <atomic>
#include "rt_common.h"
#include "rt_tsl.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#define ENABLE_MULTI_THREAD_RAY_TRACING

// scene description, all surfaces are spheres in this sample, including the wall, which are simply huge spheres.
Sphere spheres[] = {
  //Scene: radius, position,                    emission,       color,              material                        flip normal
  Sphere(  1e5,    Vec(1e5 + 1,40.8,81.6),      Vec(),          Vec(.75,.25,.25),   MaterialType::MT_Matt,            true),    //Left 
  Sphere(  1e5,    Vec(-1e5 + 99,40.8,81.6),    Vec(),          Vec(.25,.25,.75),   MaterialType::MT_Matt,            true),    //Rght 
  Sphere(  1e5,    Vec(50, 40.8, 1e5),          Vec(),          Vec(.75,.75,.75),   MaterialType::MT_Matt,            true),    //Back 
  Sphere(  1e5,    Vec(50, 40.8,-1e5 + 170),    Vec(),          Vec(),              MaterialType::MT_Matt,            true),    //Frnt 
  Sphere(  1e5,    Vec(50, 1e5, 81.6),          Vec(),          Vec(.75,.75,.75),   MaterialType::MT_Matt,            true),    //Botm 
  Sphere(  1e5,    Vec(50, 1e5 + 81.6,81.6),    Vec(),          Vec(.75,.75,.75),   MaterialType::MT_Matt,            false),   //Top 
  Sphere(  16.5,   Vec(27, 16.5,47),            Vec(),          Vec(1,1,1) * .999,  MaterialType::MT_Perlin_Matt,   false),   //Left Sphere 
  Sphere(  16.5,   Vec(73, 16.5,78),            Vec(),          Vec(1,1,1) * .999,  MaterialType::MT_Gold,            false),   //Right Sphere
  Sphere(  600,    Vec(50, 681.6 - .27,81.6),   Vec(24,24,24),  Vec(),              MaterialType::MT_Matt,            false)    //Lite 
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

// the core of path tracing algorithm
Vec radiance(Ray r) {
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

        // accumulate the radiance
        l = l + thr.mult(obj.e);

        // the position of intersection
        Vec p = r.o + r.d * t;

        // all materials are lambert, this will be replaced with TSL driven 
        auto bxdf = get_bxdf(obj, p);

        // importance sampling happens here
        Vec wi;
        float pdf = 1.0f;
        const auto ret = bxdf->sample(p, Vec(-r.d.x, -r.d.y, -r.d.z), wi, pdf);
        if (pdf <= 0.0f)
            break;
        thr = thr.mult(ret.mult(1.0f/pdf));

        // russian roulette
        ++depth;
        if (depth > 5) {
            const auto p = 0.2f;
            if (random_number() < p)
                thr = thr * (1 / p);
            else
                break;
        }

        // this adds a bit of bias, but I'm fine with it
        if (depth > 10)
            break;

        r.o = p + wi * 0.0001;
        r.d = wi;
    }

    // This is the last resort to kill fireflies in the image. It is a pretty dirty hack. 
    // Ideally, I should have fixed all cases that could have caused fireflies from its source.
    // But again, this is not the focus of the program, I'll pick the low hanging fruit.
    l.x = std::min(l.x, 10.0);
    l.y = std::min(l.y, 10.0);
    l.z = std::min(l.z, 10.0);

    return l;
}

int rt_main(int samps) {
    const auto w = 1024u, h = 768u;    // resolution of the image
    const auto inv_samps = 1.0f / samps;
    const auto total_pixel_cnt = w * h;

    // camera position and direction
    auto cam = Ray(Vec(50, 52, 295.6), Vec(0, -0.042612, -0.95).norm());
    auto cx = Vec(w * .5135 / h, 0.0f, 0.0f), cy = Vec((cx % cam.d).norm() * .5135);

    // for each pixel, take a number of samples and resolve them together.
    auto c = std::make_unique<Vec[]>(total_pixel_cnt);

#ifdef ENABLE_MULTI_THREAD_RAY_TRACING
    // the number of physical cores available in CPU
    const auto processor_count = std::thread::hardware_concurrency();

    // each thread is responsible for a bunch of lines depending on how many physical cores are available on the platform.
    const auto batch_line_number = (h + processor_count - 1) / processor_count;
    const auto batch_number = ( h + batch_line_number - 1 ) / batch_line_number;

    // thread pool
    std::vector<std::thread> threads(batch_number);

    // pixel counter
    std::atomic<int> pixel_cnt = 0;
    for (auto b = 0; b < batch_number; ++b) {
        const auto batch_offset = b * batch_line_number;
        const auto batch_max = std::min(batch_offset + batch_line_number, h);

        // spawn a separate thread for ray tracing
        threads[b] = std::thread([&](unsigned batch_offset, unsigned batch_max) {
            for (auto y = batch_offset; y < batch_max; ++y) {
                for (unsigned short x = 0; x < w; x++) {
                    auto i = (h - y - 1) * w + x;
                    auto r = Vec();
                    for (int s = 0; s < samps; s++) {
                        // make sure we have memory for allocating bxdf closures
                        reset_memory_allocator();

                        double r1 = 2.0f * random_number(), dx = r1 < 1.0f ? sqrt(r1) - 1.0f : 1.0f - sqrt(2.0f - r1);
                        double r2 = 2.0f * random_number(), dy = r2 < 1.0f ? sqrt(r2) - 1.0f : 1.0f - sqrt(2.0f - r2);
                        Vec d = cx * (((.5f + dx) / 2.f + x) / w - .5f) + cy * (((.5f + dy) / 2.f + y) / h - .5f) + cam.d;
                        r = r + radiance(Ray(cam.o + d * 140, d.norm())) * inv_samps;
                    }
                    c[i] = r;
                    ++pixel_cnt;
                }
            }
        }, batch_offset, batch_max);
    }

    // this kind of synchronization is by no means the most performant one, but it does do its job
    while (pixel_cnt < total_pixel_cnt)
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps, 100. * pixel_cnt / total_pixel_cnt);

    // flush the message and change a new line
    fprintf(stderr, "\rRendering (%d spp) %5.2f%%\n", samps, 100. * pixel_cnt / total_pixel_cnt);

    // making sure all threads are done
    std::for_each(threads.begin(), threads.end(), [](std::thread& thread) { thread.join(); });

#else
    for (int y = 0; y < h; y++) {
        fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps, 100. * y / (h - 1));
        for (unsigned short x = 0; x < w; x++) {
            auto i = (h - y - 1) * w + x;
            auto r = Vec();
            for (int s = 0; s < samps; s++) {
                // make sure we have memory for allocating bxdf closures
                reset_memory_allocator();

                double r1 = 2.0f * random_number(), dx = r1 < 1.0f ? sqrt(r1) - 1.0f : 1.0f - sqrt(2.0f - r1);
                double r2 = 2.0f * random_number(), dy = r2 < 1.0f ? sqrt(r2) - 1.0f : 1.0f - sqrt(2.0f - r2);
                Vec d = cx * (((.5f + dx) / 2.f + x) / w - .5f) + cy * (((.5f + dy) / 2.f + y) / h - .5f) + cam.d;
                r = r + radiance(Ray(cam.o + d * 140, d.norm())) * inv_samps;
            }
            c[i] = r;
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