#pragma once

#include <material.cl>
#include <triangle.cl>
#include <camera.cl>
#include <prng.cl>
#include <util.cl>

/** @file epsilon.cl
  * @brief Rendering kernel.
**/

/** This is a vector delta used for reliably computing the phase basis for
  * scattering, any nonzero vector will do since scattering is isotropic.
**/
#define VDELTA (float3)(1, 1, 1)

/** Maximum number of nested materials, setting this value too high will cause 
  * register pressure and will slow down the rendering.
**/
#define MT 10

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

typedef struct Params
{
    uint width, height;
} Params;

/* Image sampler, using texel linear interpolation. */
constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
                             CLK_ADDRESS_CLAMP_TO_EDGE  |
                             CLK_FILTER_LINEAR;

/** This is the main kernel, which performs the entire ray tracing step.
  * @param buffer The pixel buffer, as a flat 2D array.
  * @param params The render parameters (render width and height).
  * @param spectrum The tristimulus curve, to map wavelengths to colors.
  * @param triangles The list of triangles in the scene.
  * @param nodes The tree datastructure, as a list of nodes.
  * @param mapping The model to material mapping.
  * @param camera The virtual camera parameters.
  * @param seed The PRNG's seed.
**/
void kernel clmain(   global   float4        *buffer, 
                    constant   Params        *params,
                   read_only   image2d_t    spectrum, 
                      global   Triangle   *triangles, 
                      global   Node           *nodes,
                    constant   uint         *mapping,
                    constant   Camera        *camera,
                    constant   ulong4          *seed)
{
    /* Init PRNG for this worker. */
    size_t index = get_global_id(0);
    PRNG prng = init(index, seed);

    /* Jitter for antialiasing. */
    float a1 = rand(&prng) - 0.5f;
    float a2 = rand(&prng) - 0.5f;
    
    /* Obtain normalized pixel coordinates between 0 and 1 excl. */
    float x = (float)(a1 + index % params->width) /  params->width;
    float y = (float)(a2 + index / params->width) / params->height;

    /* Aspect ratio correction, prefers widescreen... */
    float ratio = (float)params->width / params->height;
    x = 0.5f * (1 + ratio) - x * ratio;

    /* The ray as vectors. */
    float3 origin, direction;

    /* Compute the camera ray from the normalized pixel coordinates. */
    Trace(x, y, &origin, &direction, rand(&prng), rand(&prng), camera);

    /* Media stack. */
    uint matStack[MT];
    uint matPos = 0;

    /* Atmospheric medium. */
    matStack[0] = mapping[0];

    /* Select random wavelength. */
    float wavelength = rand(&prng);

    /* Convert this wavelength into nanometers. */
    float w_nm = (wavelength * 400 + 380) * 1e-9f;

    /* Find light path. */
    float radiance = 0.0f;
    while (true)
    {
        /* Triangle/far point. */
        uint hit = -1; float t_d;

        /* Intersect the ray against the entire scene using the tree. */
        if (!Intersect(origin, direction, &t_d, &hit, triangles, nodes))
        {
            /* Escaped ray - implement sky system here later. */
            radiance = 0.0f;
            break;
        }

        /* Get the intersected triangle. */
        Triangle triangle = triangles[hit];

        /* Obtain the triangle's correct matID. */
        uint mappingMatID = mapping[triangle.mat];

        /* Calculate medium absorption coefficient. */
        float ke = absorption(matStack[matPos], w_nm);

        /* Expected scattering distance. */
        float s_d = -log(rand(&prng)) / ke;

        /* Scatter? */
        if (s_d < t_d)
        {
            /* Advance to scatter location. */
            origin = origin + s_d * direction;

            /* Build the phase basis (scattering is always isotropic). */
            float3 v_t = normalize(cross(direction, direction + VDELTA));
            float3 v_b = normalize(cross(direction, v_t));
            float3 v_n = direction;

            /* Rotate ray, to unit space. */
            direction = direction.x * (-v_b)
                      + direction.y * (-v_n)
                      + direction.z * (-v_t);

            /* Construct an inverse TBN matrix here. */
            float3 w_b = (float3)(v_b.x, v_n.x, v_t.x);
            float3 w_n = (float3)(v_b.y, v_n.y, v_t.y);
            float3 w_t = (float3)(v_b.z, v_n.z, v_t.z);

            /* Scatter the ray by using this material's properties. */
            float4 scattered = scatter(matStack[matPos], w_nm, &prng);
            direction = scattered.xyz;
            radiance  = scattered.w;

            /* Go back to world space. */
            direction = direction.x * v_b
                      + direction.y * v_n
                      + direction.z * v_t;
        }
        else
        {
            /* Move ray to intersection pt. */
            origin = origin + t_d * direction;

            /* Obtain TBN matrix. */
            float3 v_t = triangle.t;
            float3 v_b = triangle.b;
            float3 v_n = triangle.n;

            /* Flip the normal, with the bitangent, if necessary. */
            if (dot(v_n, direction) > 0) { v_n = -v_n; v_b = -v_b; }

            /* Construct an inverse TBN matrix here. */
            float3 w_b = (float3)(v_b.x, v_n.x, v_t.x);
            float3 w_n = (float3)(v_b.y, v_n.y, v_t.y);
            float3 w_t = (float3)(v_b.z, v_n.z, v_t.z);

            /* Transform to TBN space. */
            direction = direction.x * w_b
                      + direction.y * w_n
                      + direction.z * w_t;

            /* Check if the triangle emits light, and if so, stop. */
            radiance = exitant(mappingMatID, w_nm, direction, &prng);
            if (radiance > 0.0f) break; /* This is a light source. */

            /* Nested media? */
            bool nested = true;

            /* Select the right media at the interface. */
            uint in = matStack[matPos], to = mappingMatID;
            if (mappingMatID == matStack[matPos])
            {
                /* Leaving this medium. */
                to = matStack[matPos - 1];
                nested = false;
            }

            /* Reflect this ray, by using the reflectance function.. */
            float4 reflected = reflect(in, to, w_nm, direction, &prng,
                                       nested);
            direction = reflected.xyz;
            radiance  = reflected.w;

            /* Go back to world space. */
            direction = direction.x * v_b
                      + direction.y * v_n
                      + direction.z * v_t;

            /* Is ray reflected? */
            if (reflected.y < 0.0f)
            {
                /* Ray is transmitted. */
                origin += (-v_n) * PSHBK;

                /* Has this light ray left current material? */
                if (mappingMatID == matStack[matPos]) matPos--;
                else
                {
                    /* Else, add material to stack. */
                    matStack[++matPos] = mappingMatID;
                }
            }
            else
            {
                /* Push this ray back. */
                origin += (+v_n) * PSHBK;
            }
        }

        /* Perform adaptive russian roulette here (discard). */
        if (rand(&prng) > radiance) { radiance = 0.0f; break; }
    }

    /* Transform this spectral sample to a color using the spectral curve. */
    float3 xyz = read_imagef(spectrum, sampler, (float2)(wavelength, 0)).xyz;

    /* Accumulate this spectral sample into pixel buffer. */
    buffer[get_global_id(0)] += (float4)(xyz * radiance, 1);
}
