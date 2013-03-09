#pragma once

#include <util.cl>

/** @file triangle.cl
  * @brief Kernel triangle implementation.
**/

/** @struct Triangle
  * @brief Kernel triangle representation.
**/
typedef struct Triangle
{
    /** An arbitrary vertex of the triangle. **/
    float3 p1;
    /** First edge of the triangle. **/
    float3 e1;
    /** Second edge of the triangle. **/
    float3 e2;
    /** Triangle's tangent vector. **/
    float3 t;
    /** Triangle's bitangent vector. **/
    float3 b;
    /** Triangle's normal vector. **/
    float3 n;
    /** The triangle's material ID. **/
    uint mat;
} Triangle;

/** Performs an intersection test between a ray and a triangle.
  * @param o The ray's origin.
  * @param d The ray's direction, as a unit vector.
  * @param triangle The triangle to test intersection against.
  * @param distance A pointer to the intersection distance.
  * @returns Returns \c true if an intersection exists, \c false otherwise.
  *          If this function returns \c false, then \c *distance is
  *          indeterminate and must not be used.
**/
bool RayTriangle(float3 o, float3 d, Triangle triangle, float *distance)
{
    o -= triangle.p1.xyz;
    float3 s = cross(d, triangle.e2.xyz);
    float de = 1.0 / dot(s, triangle.e1.xyz);

    float u = dot(o, s) * de;

    if ((u <= -EPSILON) || (u >= 1 + EPSILON)) return false;

    s = cross(o, triangle.e1.xyz);
    float v = dot(d, s) * de;

    if ((v <= -EPSILON) || (u + v >= 1 + EPSILON)) return false;

    *distance = dot(triangle.e2.xyz, s) * de;
    return (*distance > EPSILON);
}
