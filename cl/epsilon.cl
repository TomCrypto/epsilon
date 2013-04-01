#pragma once

/* UNCOMMENT THIS FOR NOACCEL MODE, this mode discards all scene information, *
 * including geometry data, BVH nodes, and camera parameters, then falls back *
 * to a user-defined scene in noaccel.cl, by using spheres. Tree acceleration *
 * is also disabled. This is useful for testing which doesn't require complex *
 * geometry, giving some extra performance by eliminating traversal overhead. */
#define KERNEL_MODE_NOACCEL

#ifdef KERNEL_MODE_NOACCEL
#include <noaccel.cl>
#endif

#include <material.cl>
#include <camera.cl>
#include <prng.cl>
#include <util.cl>
#include <bvh.cl>

/** @file epsilon.cl
  * @brief Rendering kernel.
**/

/** Maximum number of nested materials, setting this value too high will cause 
  * register pressure and will slow down the rendering.
**/
#define MT 4

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

    #ifdef KERNEL_MODE_NOACCEL
    /* Trace camera ray with sphere scene. */
    NoAccel_Trace(x, y, &origin, &direction);
    #else
    /* Compute the camera ray from the normalized pixel coordinates. */
    Trace(x, y, &origin, &direction, rand(&prng), rand(&prng), camera);
    #endif

    /* Media stack. */
    uint matStack[MT];
    uint matPos = 0;

    #ifdef KERNEL_MODE_NOACCEL
    /* Use the scene's atmosphere. */
    matStack[0] = NOACCEL_ATMOSPHERE;
    #else
    /* Atmospheric medium. */
    matStack[0] = mapping[0];
    #endif

    /* Select random wavelength. */
    float wavelength = rand(&prng);

    /* Convert this wavelength into nanometers. */
    float w_nm = (wavelength * 400 + 380) * 1e-9f;

    /* Find light path. */
    float radiance = 0.0f;
    while (true)
    {
        /* Object hit and far point. */
        uint hit = (uint)-1; float t_d;

        #ifdef KERNEL_MODE_NOACCEL
        /* Intersect the ray against the test sphere scene. */
        if (!NoAccel_Intersect(origin, direction, &t_d, &hit))
        #else
        /* Intersect the ray against the entire scene using the tree. */
        if (!Intersect(origin, direction, &t_d, &hit, triangles, nodes))
        #endif
        {
            /* Escaped ray - implement sky system here later. */
            radiance = 0.0f;
            break;
        }

        #ifdef KERNEL_MODE_NOACCEL
        /* Get the intersected sphere material. */
        uint mappingMatID = spheres[hit].material;
        #else
        /* Get the intersected triangle. */
        Triangle triangle = triangles[hit];

        /* Obtain the triangle's correct matID. */
        uint mappingMatID = mapping[triangle.mat];
        #endif

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

            /* Compute the inverse phase basis here. */
            float3 w_b = (float3)(v_b.x, v_n.x, v_t.x);
            float3 w_n = (float3)(v_b.y, v_n.y, v_t.y);
            float3 w_t = (float3)(v_b.z, v_n.z, v_t.z);

            /* Rotate into unit space. */
            direction = direction.x * w_b
                      + direction.y * w_n
                      + direction.z * w_t;


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

            #ifdef KERNEL_MODE_NOACCEL
            /* Get the sphere normal, at the intersection. */
            float3 v_n = ComputeNormal(origin, spheres[hit]);
            float3 v_t = normalize(cross(v_n, v_n + VDELTA));
            float3 v_b = normalize(cross(v_n, v_t));
            #else
            /* Obtain TBN matrix. */
            float3 v_t = triangle.t;
            float3 v_b = triangle.b;
            float3 v_n = triangle.n;
            #endif

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

            /* Reflect this ray, using this material's reflectance function. */
            float4 reflected = reflect(in, to, w_nm, direction, &prng, nested);
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
