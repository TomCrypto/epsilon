#pragma once

#include <material.cl>
#include <sphere.cl>
#include <util.cl>

/** @file noaccel.cl
  * @brief High-performance testing utilities.
  *
  * This file provides high-performance (but limited) utility functions to test
  * the kernel when complex geometry is not needed. It uses spheres, instead of
  * triangles, to represent volumes, and the camera model is bare bones.
**/

/* Select atmosphere material. */
#define NOACCEL_ATMOSPHERE VACUUM

/* Sphere count. */
#define SPHERE_CT 8

/* This is the sphere list, put in as many as you want however be aware scene *
 * traversal cost will increase linearly, as no acceleration at all is used.  */
constant Sphere spheres[SPHERE_CT] = {

    { (float4)(1e4f + 1, 40.8f, 81.6f, 1e8f), DIFFUSE_BLUE },
    { (float4)(-1e4f + 99, 40.8f, 81.6f, 1e8f), DIFFUSE_GREEN },
    { (float4)(50, 40.8f, 1e4f, 1e8f), DIFFUSE_WHITE },
    { (float4)(50, 40.8f, -1e4f + 270, 1e8f), DIFFUSE_WHITE },
    { (float4)(50, 1e4f, 81.6f, 1e8f), DIFFUSE_WHITE },
    { (float4)(50, -1e4f + 81.6f, 81.6f, 1e8f), DIFFUSE_WHITE },
    { (float4)(50, 81.6f - 15 + 50, 81.6f, 1600), WHITE_FLUORESCENT },
    { (float4)(55, 16.55f, 78, 272.25f), GLASS_RED_WEAK }
};

/* These are the camera settings, namely position, target, and field of view. */
constant float3 NoAccel_cameraPos = (float3)(50, 45, 205.6f);
constant float3 NoAccel_cameraTar = (float3)(50, 45 - 0.042612f, 204.6f);
constant float  NoAccel_cameraFOV = 45; /* Degrees. */

/** Traces a camera ray according to the test scene camera settings.
  * @param u The normalized x-coordinate.
  * @param v The normalized y-coordinate.
  * @param origin A pointer to the camera ray's origin.
  * @param direction A pointer to the camera ray's direction.
**/
void NoAccel_Trace(float u, float v, float3 *origin, float3 *direction)
{
    float3 dir = normalize(NoAccel_cameraTar - NoAccel_cameraPos);
    float3 v_n = (float3)(0.0f, 1.0f, 0.0f);
    float3 v_b = normalize(cross(dir, v_n));
    float3 v_t = dir;

    float fov = NoAccel_cameraFOV * (PI / 180.0f);
    float3 ray = (float3)(1 - 2 * u, 1 - 2 * v, 1.0f / tan(fov * 0.5f));

    *direction = normalize(ray.x * v_b + ray.y * v_n + ray.z * v_t);
    *origin = NoAccel_cameraPos;
}

/** Provides ray-scene intersection for the defined sphere scene.
  * @param origin The ray's origin.
  * @param direction The ray's direction.
  * @param t A pointer to a \c float in which to store the nearest intersection
  *          distance.
  * @param hit A pointer to a \c uint in which to store the nearest sphere
  *            intersected.
  * @param spheres A list of spheres to check intersection against.
  * @returns A boolean indicating whether the ray intersects a sphere or not.
  *          If this function returns \c false, the values of \c *t and \c *hit
  *          are indeterminate and should not be used.
**/
bool NoAccel_Intersect(float3 origin, float3 direction, float *t, uint *hit)
{
    *hit = (uint)-1;
    *t = INFINITY;

    #pragma unroll
    for (uint n = 0; n < SPHERE_CT; ++n)
    {
        float d = RaySphere(origin, direction, spheres[n]);
        if ((d >= EPSILON) && (d < *t))
        {
            *hit = n;
            *t = d;
        }
    }

    return (*hit != (uint)-1);
}

