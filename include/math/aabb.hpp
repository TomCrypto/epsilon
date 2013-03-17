#pragma once

#include <math/vector.hpp>

#ifdef _WIN32
#undef near
#undef far
#endif

/** @file aabb.hpp
  * @brief Bounding box manipulation.
  *
  * This file contains methods to work on axis-aligned bounding boxes.
**/

/** @brief Axis-aligned bounding box.
**/
struct AABB
{
    /** @brief Minimum corner. **/
    Vector min;
    /** @brief Maximum corner. **/
    Vector max;
    /** @brief Diagonal extent, defined as \c max \c - \c min. **/
    Vector extent;

    /** @brief Constructs the zero bounding box. **/
    AABB() { }

    /** @brief Constructs a bounding box containing a single point.
      * @param p The point (as a vector) the bounding box is to contain.
    **/
    AABB(const Vector& p) : min(p), max(p) { }

    /** @brief Constructs a bounding box from two corners.
      * @param min The minimum corner.
      * @param max The maximum corner.
    **/
    AABB(const Vector& min, const Vector& max) : min(min), max(max)
    {
        extent = max - min;
    }

    /** @brief Expands this bounding box to contain an arbitrary point.
      * @param p The point (as a vector) this bounding box is to contain.
      * @note If \c p is already in this bounding box, does nothing.
    **/
    void ExpandToInclude(const Vector& p)
    {
        min = vmin(min, p);
        max = vmax(max, p);
        extent = max - min;
    }

    /** @brief Expands this bounding box to contain an arbitrary bounding box.
      * @param b The bounding box this bounding box is to contain.
      * @note If \c b is already fully contained in this bounding box, this
      *       method does nothing.
    **/
    void ExpandToInclude(const AABB& b)
    {
        min = vmin(min, b.min);
        max = vmax(max, b.max);
        extent = max - min;
    }
   
    /** @brief Returns the dimension this bounding box is largest in.
      * @return If this bounding box is largest in the \c x dimension, returns
      *         \c 0, if it is largest in the \c y dimension, returns \c 1,
      *         and so on.
      * @note This is a very specialized method which is notably used by the
      *        BVH construction heuristic.
    **/
    uint32_t Split() const
    {
        uint32_t result = 0;
        if (extent.y > extent.x) result = 1;
        if (extent.z > extent.y) result = 2;
        return result;
    }

    /** @brief Ray-AABB intersection test.
      * @param origin The origin of the ray to test against.
      * @param direction The (unit) direction of the ray to test against.
      * @param near A pointer to the near intersection distance (closest).
      * @param far A pointer to the far intersection distance (furthest).
      * @return Returns \c true if an intersection occurs, and \c false
      *         otherwise. If this method returns \c false, then the values
      *         of \c *near and \c *far are indeterminate.
    **/
    bool Intersect(const Vector& origin, const Vector& direction,
                   float *near, float *far)
    {
        Vector bot = (min - origin) / direction;
        Vector top = (max - origin) / direction;

        Vector tmin = vmin(top, bot);
        Vector tmax = vmax(top, bot);

        *near = std::max(std::max(tmin.x, tmin.y), tmin.z);
        *far  = std::min(std::min(tmax.x, tmax.y), tmax.z);

        return !(*near > *far) && *far > 0;
    }
};
