#pragma once

#include <util.cl>

/** @file sphere.cl
  * @brief Kernel sphere implementation.
**/

/** @struct Sphere
  * @brief Kernel sphere representation.
**/
typedef struct Sphere
{
    /** The sphere's center, and squared radius, as (x, y, z, r^2). **/
    float4 position;
    /** The sphere's material, no mapping will be performed. **/
    uint material;
} Sphere;

/** Intersects a ray with a sphere.
  * @param origin The ray's origin.
  * @param direction The ray's direction.
  * @param sphere The sphere to intersect the ray against.
  * @returns A floating-point number indicating the distance to intersection.
  *          If the returned value is negative, the ray does not intersect the
  *          sphere. Note this returns the nearest intersection distance.
**/
float RaySphere(float3 origin, float3 direction, Sphere sphere)
{
    origin = sphere.position.xyz - origin;
    float b = dot(origin, direction); /* w -> radius squared. */
    float det = b * b - dot(origin, origin) + sphere.position.w;
    if (det >= 0.0f)
    {
        float l = sqrt(det);
        float p = b - l, q = b + l;
        if (p < EPSILON) return q;
        if (q < EPSILON) return p;
        return min(p, q);
    }
    
    return -1.0f;
}

/** Returns the sphere's normal at a given point on the sphere's surface.
  * @param point The point on the sphere's surface.
  * @param sphere The relevant sphere.
  * @returns A normal vector corresponding to the sphere's surface normal at
  *          the desired point.
**/
float3 ComputeNormal(float3 point, Sphere sphere)
{
    return normalize(point - sphere.position.xyz);
}
