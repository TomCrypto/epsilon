/* Helper methods for ray/triangle intersection. */

#include <util.cl>

/* This triangle record is used solely for rendering. BVH construction is done
 * host-side and requires some more information, such as the triangle centroid
 * and bounding volume, however those variables are not required here. */
typedef struct Triangle
{
    //float3 p1, p2, n; /* Two vertices of the triangle and its face normal. */
    //float3 ln, rn, fn; /* Left, right and far edge normals. */
	//int material; // TEMPORARY
    float3 p1, e1, e2, n;
    float mat;
} Triangle;

/* Intersects a ray with a triangle. */
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

    # if 0
    /* Compute intersection with the plane in which the triangle lies. */
    *distance = dot(triangle.p1 - o, triangle.n) / dot(d, triangle.n);
    if (*distance < EPSILON) return false;

    /* Get the intersection point. */
    float3 p = o + *distance * d;
    float3 l1 = p - triangle.p1;
    float3 l2 = p - triangle.p2;

    /* Check if there exists an  intersection. */
    return ((dot(l1, triangle.ln) > -EPSILON)
         && (dot(l1, triangle.rn) > -EPSILON)
         && (dot(l2, triangle.fn) > -EPSILON));
    #endif
}
