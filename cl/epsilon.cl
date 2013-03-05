#include <camera.cl>
#include <triangle.cl>
#include <prng.cl>

typedef struct __attribute__ ((packed)) Node
{
	float4 min, max;
	uint4 data;
} Node;

bool RayBBox(float3 origin, float3 direction, float3 min, float3 max, float* near, float* far)
{
	float3 bot = (min - origin) / direction;
	float3 top = (max - origin) / direction;

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

bool Intersect(float3 origin, float3 direction, float* distance, int* hit,
			   global Triangle* triangles, global Node* nodes)
{
	*distance = INFINITY;
	*hit = -1;

	float bbhits[4];
	int closer, other;

	BVHTraversal todo[64];
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

typedef struct __attribute__ ((packed)) Params
{
	uint width, height, pass;
} Params;

typedef struct __attribute__ ((packed)) SceneInfo
{
	uint triangleCount;
} SceneInfo;

const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
						  CLK_ADDRESS_CLAMP_TO_EDGE  |
                          CLK_FILTER_LINEAR;

#if 0

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

#endif

void kernel clmain(global float4 *render, /* Render buffer in XYZn format. */
				   constant Params *params, /* Scene information. */
				   read_only image1d_t spectrum, /* Color-matching function. */
	     		   global Triangle *triangles, /* List of triangles in scene. */
	     		   global Node *nodes, /* List of BVH nodes in scene. */
				   constant SceneInfo *sceneInfo, /* Scene information. */
				   constant Camera *camera, /* Camera information. */
				   constant ulong4 *seed /* Seed for the PRNG. */
                   )
{
	/* TEMPORARY | matID -> wavelength peak colors. negative -> light. */
	float matID[6] = {0, 0, 525, 480, -1, 640};

	size_t index = get_global_id(0);

	/* Get a PRNG instance for this worker. */
    PRNG prng = init(index, seed);

    /* Get pixel coordinates in normalized coordinates [0..1). */
    float a1 = rand(&prng) - 0.5f;
    float a2 = rand(&prng) - 0.5f;

	float ratio = (float)params->width / params->height;
	float x = (float)(a1 + index % params->width) / params->width;
	float y = (float)(a2 + index / params->width) / params->height;
	x = 0.5f * (1 + ratio) - x * ratio;

    /* Compute the standard camera ray. */
    float3 origin, direction;
    Trace(x, y, &origin, &direction, camera, rand(&prng), rand(&prng));

    /* Select random wavelength. */
    float wavelength = rand(&prng);
	//float wavelength = (params->pass % 81) / 80.0f;

    /* Repeat until we die. */
    float radiance = 0.0f;
    while (true)
    {
		int hit = -1;
        Triangle tri;

		float t;
		bool X = Intersect(origin, direction, &t, &hit, triangles, nodes);

        if (!X)
        {
			radiance = 10;
            break;
        }

		tri = triangles[hit];
		float material = matID[tri.mat];

        float3 point = origin + t * direction;

        /* Check material for light! */
        if (material < 0)
        {
            /* This is a light. */
			float w = (380 + 400 * wavelength) * 1e-9;
			float powerTerm  = 3.74183e-16f * pow(w, -5.0f);
    		radiance = powerTerm / (exp(1.4388e-2f / (w * 4150)) - 1.0f);
            break;
        }

        /* Get normal. */
        float3 normal = tri.n;
        normal = normal * sign(-dot(normal, direction));

		/* Get real wavelength (380nm to 680nm) */
		float w = 380 + 400 * wavelength;

		/* Otherwise, apply response curve. */
		float response = (material == 0) ? 1.0 : 0.45 + exp(-pow(w - material, 2.0f) * 0.001f) * 0.55;
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
