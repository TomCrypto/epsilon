#pragma once

/* This file contains an implementation of axis-aligned bounding box, used in *
 * the bounding volume hierarchy code. This structure also exists device-side *
 * as it is used during bounding volume hierarchy traversal.                  */

#include <math/vector.hpp>

struct cl_aabb
{
    cl_float4 min; /* Minimum AABB corner. */
    cl_float4 max; /* Maximum AABB corner. */
};

struct AABB
{
    Vector min, max;
    Vector extent;

    AABB() { }
    AABB(const Vector& p) : min(p), max(p) { }
    AABB(const Vector& min, const Vector& max) : min(min), max(max)
    {
        this->extent = this->max - this->min;
    }


    void ExpandToInclude(const Vector& p)
    {
        this->min = vmin(this->min, p);
        this->max = vmax(this->max, p);
        this->extent = this->max - this->min;
    }

    void ExpandToInclude(const AABB& b)
    {
        this->min = vmin(this->min, b.min);
        this->max = vmax(this->max, b.max);
        this->extent = this->max - this->min;
    }
   
    uint32_t Split() const
    {
        uint32_t result = 0;
        if (this->extent.y > this->extent.x) result = 1;
        if (this->extent.z > this->extent.y) result = 2;
        return result;
    }

    bool Intersect(const Vector& origin, const Vector& direction,
                   float *near, float *far)
    {
        Vector bot = (this->min - origin) / direction;
        Vector top = (this->max - origin) / direction;

        Vector tmin = vmin(top, bot);
        Vector tmax = vmax(top, bot);

        *near = std::max(std::max(tmin.x, tmin.y), tmin.z);
        *far  = std::min(std::min(tmax.x, tmax.y), tmax.z);

        return !(*near > *far) && *far > 0;
    }
};
