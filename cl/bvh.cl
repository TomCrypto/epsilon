#pragma once

#include <triangle.cl>

/** @file bvh.cl
  * @brief Kernel BVH Traversal.
**/

typedef struct Node
{
    float4 min, max;
    uint4 data;
} Node;

/** Computes the intersection between a ray and a bounding box.
  * @param origin The ray's origin.
  * @param direction The ray's direction, as a unit vector.
  * @param bmin The bounding box's minimum coordinate.
  * @param bmax The bounding box's maximum coordinate.
  * @param near A pointer to the near intersection distance.
  * @param far A pointer to the far intersection distance.
  * @returns Whether an intersection occurs between the ray and the bounding
  *          box. If this is \c true, the values of \c *near and \c *far 
  *          indicate the distance to intersection. If this is \c false,
  *          the values of \c *near and \c *far are indeterminate.
**/
bool RayBBox(float3 origin, float3 direction,
             float3 bmin, float3 bmax,
             float* near, float* far)
{
    float3 bot = (bmin - origin) / direction;
    float3 top = (bmax - origin) / direction;

    float3 tmin = fmin(top, bot);
    float3 tmax = fmax(top, bot);

    *near = fmax(fmax(tmin.x, tmin.y), tmin.z);
    *far  = fmin(fmin(tmax.x, tmax.y), tmax.z);

    return !(*near > *far) && *far > 0;
}

//! Node for storing state information during traversal.
typedef struct BVHTraversal {
 uint i; // Node
 float mint; // Minimum hit time for this node.
} BVHTraversal;

BVHTraversal MakeTraversal(int i, float mint)
{
    BVHTraversal ret;
    ret.i = i;
    ret.mint = mint;
    return ret;
}

bool Intersect(float3 origin, float3 direction, float* distance, uint *hit,
               global Triangle* triangles, global Node* nodes)
{
    *distance = INFINITY;
    *hit = -1;

    float bbhits[4];
    int closer, other;

    BVHTraversal todo[24];
    int stackptr = 0;

    todo[stackptr].i = 0;
    todo[stackptr].mint = -INFINITY;

    while (stackptr >= 0)
    {
        int ni = todo[stackptr].i;
        float near = todo[stackptr].mint;
        stackptr--;

        Node node = nodes[ni];

        if(near > *distance) continue;

        if (node.data.z == 0)
        {
            for (int o = 0; o < node.data.y; ++o)
            {
                float dist;
                Triangle tri = triangles[node.data.x + o];
                bool intersects = RayTriangle(origin, direction, tri, &dist);
                if (intersects && (dist < *distance))
                {
                    *distance = dist;
                    *hit = node.data.x + o; // wtf
                }
            }
        }
        else
        {
            bool hitc0 = RayBBox(origin, direction, nodes[ni + 1].min.xyz, nodes[ni + 1].max.xyz, bbhits, bbhits + 1);
            bool hitc1 = RayBBox(origin, direction, nodes[ni + node.data.z].min.xyz, nodes[ni + node.data.z].max.xyz, bbhits + 2, bbhits + 3);

            if (hitc0 && hitc1)
            {
                closer = ni + 1;
                other = ni + node.data.z;

                if (bbhits[2] < bbhits[0])
                {
                    float tmp = bbhits[0];
                    bbhits[0] = bbhits[2];
                    bbhits[2] = tmp;

                    tmp = bbhits[1];
                    bbhits[1] = bbhits[3];
                    bbhits[3] = tmp;

                    int tmp2 = closer;
                    closer = other;
                    other = tmp2;
                }

                todo[++stackptr] = MakeTraversal(other, bbhits[2]);
                todo[++stackptr] = MakeTraversal(closer, bbhits[0]);
            }
            else if (hitc0)
            {
                todo[++stackptr] = MakeTraversal(ni + 1, bbhits[0]);
            }
            else if (hitc1)
            {
                todo[++stackptr] = MakeTraversal(ni + node.data.z, bbhits[2]);
            }
        }
    }

    return (*hit != -1);
}
