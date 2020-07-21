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

#include "rt_common.h"

// bxdf is the basic of surface interaction.
class Bxdf {
public:
    Bxdf(const Vec center, const bool fn) :sphere_center(center), flip_normal(fn) {}
    virtual ~Bxdf() {}

    // take sample based on bxdf
    virtual Vec sample(const Vec& pos, const Vec& wo, Vec& wi, float& pdf) = 0;

protected:
    const Vec   sphere_center;
	const bool	flip_normal;

    // helper method to convert vector between world space and local space
    Vec local_to_world(const Vec& pos, const Vec& vec);
    Vec world_to_local(const Vec& pos, const Vec& vec);
};

// very standard lambertian brdf model
class Lambert : public Bxdf{
public:
    Lambert(const Vec color, const Vec center, const bool fn) 
        :basecolor(color), Bxdf(center, fn) {}
    Vec sample(const Vec& pos, const Vec& wo, Vec& wi, float& pdf) override;

private:
    const Vec basecolor;
};

class Microfacet : public Bxdf {
public:
    Microfacet(const Vec center, const bool fn)
        :Bxdf(center, fn) {}

    Vec sample(const Vec& pos, const Vec& wo, Vec& wi, float& pdf) override;
};