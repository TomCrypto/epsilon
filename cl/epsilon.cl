#include <camera.cl>
#include <triangle.cl>
#include <prng.cl>

typedef struct __attribute__ ((packed)) Params
{
	uint width, height;
} Params;

typedef struct __attribute__ ((packed)) SceneInfo
{
	uint triangleCount;
} SceneInfo;

const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
						  CLK_ADDRESS_CLAMP_TO_EDGE  |
                          CLK_FILTER_LINEAR;

float Intersect(global Triangle *geometry, float3 origin, float3 direction,
                int *hit, uint count)
{
    float min_t = INFINITY;
    int closest = -1;

    for (uint i = 0; i < count; ++i)
    {
        float dist;
        if (RayTriangle(origin, direction, geometry[i], &dist))
        {
            if (dist < min_t)
            {
                min_t = dist;
                closest = i;
            }
        }
    }

    if (closest != -1) *hit = closest;
    return min_t;
};

void kernel clmain(global float4 *render, /* Render buffer in XYZn format. */
				   constant Params *params, /* Scene information. */
				   read_only image1d_t spectrum, /* Color-matching function. */
	     		   global Triangle *geometry, /* List of triangles in scene. */
				   constant SceneInfo *sceneInfo, /* Scene information. */
				   constant Camera *camera, /* Camera information. */
				   constant ulong4 *seed /* Seed for the PRNG. */
                   )
{
	/* Get a PRNG instance for this worker. */
    PRNG prng = init(get_global_id(0), seed);

    /* Get pixel coordinates in normalized coordinates [0..1). */
    float a1 = rand(&prng) - 0.5f;
    float a2 = rand(&prng) - 0.5f;

	float x = (float)(a1 + get_global_id(0) % params->width) / params->width;
	float y = (float)(a2 + get_global_id(0) / params->width) / params->height;

    /* Compute the standard camera ray. */
    float3 origin, direction;
    Trace(x, y, &origin, &direction, camera);

    /* Select random wavelength. */
    float wavelength = rand(&prng);

    /* Repeat until we die. */
    float radiance = 0.0f;
    while (true)
    {
        int hit = -1;
        Triangle tri;
        float t = Intersect(geometry, origin, direction, &hit, sceneInfo->triangleCount);
        if (hit == -1)
        {
            radiance = 10.0f;
            break;
        }

        tri = geometry[hit];

        float3 point = origin + t * direction;

        /* Check material for light! */
        if (tri.mat < 0)
        {
            /* This is a light. */
            radiance = 10.0f;
            break;
        }

        /* Get normal. */
        float3 normal = tri.n;
        normal = normal * sign(-dot(normal, direction));

		/* Otherwise, apply response curve. */
		float w = 380 + 400 * wavelength;

		float response = (tri.mat == 0) ? 1.0 : exp(-pow(w - tri.mat, 2.0f) * 0.001f);
		response *= 0.8;

		/* Get a random diffuse sample. */
		float u1 = rand(&prng);
		float u2 = rand(&prng);
		float theta = 2.0f * 3.149159265 * u2;
		float r = sqrt(u1);

		float3 newdir = (float3)(r * cos(theta), sqrt(1.0f - u1), r * sin(theta));

		/* Rotate according to basis. */
		float3 basisY = normal;
		float3 UP = normalize((float3)(0.01, 0.99, 0.01));
		float3 basisX = normalize(cross(basisY, UP));
    	float3 basisZ = normalize(cross(basisX, basisY));
	    newdir = newdir.x * basisX
               + newdir.y * basisY
		       + newdir.z * basisZ;

        radiance = response;
		direction = newdir;            
        
        if (rand(&prng) > radiance)
        {
            radiance = 0.0f;
            break;
        }

        /* Else continue. */
        origin = point + normal * 0.01f;
    }

    float3 xyz = radiance * read_imagef(spectrum, sampler, wavelength).xyz;

    render[get_global_id(0)] += (float4)(xyz, radiance);
}
