#include <triangle.cl>
#include <geometry.cl>
#include <camera.cl>
#include <prng.cl>

#include <noise.cl>

const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE |
						  CLK_ADDRESS_CLAMP_TO_EDGE  |
                          CLK_FILTER_LINEAR;

typedef struct Light
{
	float3 position;
	int material;
} Light;

typedef struct __attribute__ ((packed)) Scene
{
	uint4 res;
} Scene;

float SphereIntersect(float3 origin, float3 direction, float3 center, float radius)
{
	float3 s = center - origin;
    float sd = dot(s, direction);
    float ss = dot(s, s);

    float disc = sd * sd - ss + radius * radius;

    if (disc < 0.f) return -1.0f;

    float p1 = sd - sqrt(disc);
    float p2 = sd + sqrt(disc);
    if (p1 < 0) return p2;
    if (p2 < 0) return p1;
    return fmin(p1, p2);
}

float Intersect(global Triangle *geometry, float3 origin, float3 direction,
                int *hit)
{
    float min_t = INFINITY;
    int closest = -1;

    for (int i = 0; i < 9; ++i)
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

	float dist = SphereIntersect(origin, direction, (float3)(1.7, -2.475, 0), 2.5);
	if ((dist > 0) && (dist < min_t))
	{
		min_t = dist;
		closest = 8;
	}

    if (closest != -1) *hit = closest;
    return min_t;
};

/* geometry: the entire list of triangles in the scene.
 * 


*/
void kernel clmain(global float4 *render, /* Render buffer in XYZn format. */
	     		   global Triangle *geometry, /* List of triangles in scene. */
                   //global BVHNode *bvhNodes, /* List of BVH nodes in scene. */
                   //global Model *model, /* List of models in scene. */
                   //global ModelNode *modelNodes, /* Model BVH structure. */
				   //global Light *lights, /* List of light sources. */
                   read_only image1d_array_t surface, /* Surface materials.. */
                   //read_only image1d_array_t volume, /* Volume materials. */
				   constant ulong4 *seed, /* Seed for the PRNG. */
				   read_only image1d_t spectrum, /* Color-matching function. */
				   constant Scene *scene, /* Scene information. */
                   constant Camera *camera /* Camera information. */
                   )
{
	/* Get a PRNG instance for this worker. */
    PRNG prng = init(get_global_id(0), seed);

    /* Get pixel coordinates in normalized coordinates [0..1). */
    float a1 = rand(&prng) - 0.5f;
    float a2 = rand(&prng) - 0.5f;

	float x = (float)(a1 + get_global_id(0) % scene->res.x) / scene->res.x;
	float y = (float)(a2 + get_global_id(0) / scene->res.x) / scene->res.y;

    // render[get_global_id(0)] = (float4)(read_imagef(surface, sampler,
    //                                       (float2)(x, 0)).x, 0, 0, 1.0f);
    //return;

    /* Compute the standard camera ray. */
    float3 origin, direction;
    Trace(x, y, &origin, &direction, camera);

    /* Select random wavelength. */
    float wavelength = rand(&prng);

    /* Repeat until we die. */
	bool inSphere = false;
    float radiance = 0.0f;
    while (true)
    {
        int hit = -1;
        Triangle tri;
        float t = Intersect(geometry, origin, direction, &hit);
        if (hit == -1)
        {
            radiance = 10.0f; // 0.0f;
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

		if (tri.mat == 3)
		{
			inSphere = !inSphere;
			#if 0
			normal = normalize(point - (float3)(0, 0, 0));
			point += normal * sign(dot(direction, normal)) * 0.02f;
			if (!inSphere)
			{
				float w = (wavelength * 400 + 380); float peak = 650.0f;
				radiance = exp(-t * 0.5 * (1 -exp(-pow(w - peak, 2) * 0.002f)));
			}
			else
			{
				radiance = 1.0f;
			}

			#endif

			/* Find (phi, theta) coordinates. */
			normal = normalize(point - (float3)(1.7, -2.475, 0));

			#if 0

			float phi = atan2(point.z, point.x) / (2 * 3.14159265f);
			float theta = acos(point.y / 3.5) / 3.14159265f;
			float scale = 10; //24.0f;

			float noise = snoise((float2)(phi, theta) * scale) - 1.0f;
			float DD = 500 + noise * noise * 200;

			#endif

			# if 0

			float oil = 1.3333f; float depth = DD * 1e-9;
			float w = (wavelength * 400 + 380) * 1e-9;

			float IOR = 1.0f / oil;
			float f = Fresnel(1, oil, direction, normal);
			float cosI = fabs(dot(normal, direction));
			float cosT = sqrt(1 - IOR * IOR * (1 - cosI * cosI));

			/* Get the two (intensity, phase) pairs. */
			float I1 = f; float P1 = 3.14159265;
			float I2 = (1 - f);
			float P2_refl = (2 * oil * depth * cosT) / w;
			float P2_tran = (2 * oil * depth * cosT) / w;

			P2_refl = 2 * acos(2 * (P2_refl - floor(P2_refl)) - 1);
			P2_tran = 2 * acos(2 * (P2_tran - floor(P2_tran)) - 1);

			/* Compute the new intensity. */
			float refl_radiance = I1 + I2 + 2 * sqrt(I1 * I2) * cos(P1 - P2_refl);
			float tran_radiance = I1 + I2 + 2 * sqrt(I1 * I2) * cos(P1 - P2_tran);

			#endif

			float oil = 1.3333f; float depth = 560 * 1e-9;
			float w = (wavelength * 400 + 380) * 1e-9;

			#if 0

			float IOR = 1.0f / oil;
			float f = Fresnel(1, oil, direction, normal);
			float cosI = fabs(dot(normal, direction));
			float cosT = sqrt(1 - IOR * IOR * (1 - cosI * cosI));

			/* Get the two (intensity, phase) pairs. */
			float I1 = f; float P1 = 3.14159265;
			float I2 = (1 - f);
			float P2_refl = (2 * oil * depth * cosT) / w;
			float P2_tran = (2 * oil * depth * cosT) / w;

			P2_refl = 2 * 3.14159265 * P2_refl;
			P2_tran = 2 * 3.14159265 * P2_tran;

			/* Compute the new intensity. */
			float refl_radiance = I1 + I2 + 2 * sqrt(I1 * I2) * cos(P1 - P2_refl);
			float tran_radiance = I1 + I2 + 2 * sqrt(I1 * I2) * cos(P1 - P2_tran);

			#endif

			float n0 = 1.0f; float n1 = 1.3333f; float n2 = 1.0f;

			float cos0 = fabs(dot(normal, direction));
			float cos1 = sqrt(1 - (n0 / n1) * (n0 / n1) * (1 - cos0 * cos0));
			float cos2 = sqrt(1 - (n1 / n2) * (n1 / n2) * (1 - cos1 * cos1));

			float phi = 2 * 3.14159265 / w * (2.0f * n1 * depth * cos1);
			if (n1 < n0) phi += 3.14159265;
			if (n1 < n2) phi += 3.14159265;

			float i_r = 0.0f; float i_t = 0.0f;
			float rho_0_1, rho_1_0, rho_1_2, tau_1_0, tau_0_1, tau_1_2, alpha, beta;

			if (rand(&prng) < 0.5f)
			{
				/* p-polarized. */
				rho_0_1 = rp(n0, n1, cos0, cos1);
				rho_1_0 = rp(n1, n0, cos1, cos0);
				rho_1_2 = rp(n1, n2, cos1, cos2);

				tau_1_0 = tp(n1, n0, cos1, cos0);
				tau_0_1 = tp(n0, n1, cos0, cos1);
				tau_1_2 = tp(n1, n2, cos1, cos2);

				alpha = rho_1_0 * rho_1_2;
				beta  = tau_0_1 * tau_1_2;
			}
			else
			{
				/* s-polarized. */
				rho_0_1 = rs(n0, n1, cos0, cos1);
				rho_1_0 = rs(n1, n0, cos1, cos0);
				rho_1_2 = rs(n1, n2, cos1, cos2);

				tau_1_0 = ts(n1, n0, cos1, cos0);
				tau_0_1 = ts(n0, n1, cos0, cos1);
				tau_1_2 = ts(n1, n2, cos1, cos2);

				alpha = rho_1_0 * rho_1_2;
				beta  = tau_0_1 * tau_1_2;
			}

			/* float transmission = (1 - Fresnel2(n0, n1, cos0, cos1)) * (1 - Fresnel2(n1, n2, cos1, cos2));
			float reflection = Fresnel2(n1, n0, cos1, cos0) * Fresnel2(n1, n2, cos1, cos2);

			i_t = transmission / (reflection + 1 - 2 * sqrt(reflection) * cos(phi)); */

			i_t = pow(beta, 2.0f) / (pow(alpha, 2.0f) - 2 * alpha * cos(phi) + 1);

			//i_r = pow(beta * rho_1_2 + alpha * rho_1_0 + rho_0_1 * cos(phi), 2);

			//i_r /= (pow(alpha, 2.0f) - 2 * alpha * cos(phi) + 1);

			i_r = 1.0f - i_t;


			/* Get new direction. */
			//direction = direction - 2 * normal * dot(direction, normal);
			if (rand(&prng) < i_r)
			{
				/* Reflected. */
				direction = direction - 2 * normal * dot(direction, normal);
				radiance = 1.0f;
			}
			else if (rand(&prng) < i_t)
			{
				/* Refracted. */
				//point += direction * 0.05f;
				point += normal * sign(dot(direction, normal)) * 0.015f;
				radiance = 1.0f;
			}
			else break;
		}
		else
		{
		    /* Otherwise, apply response curve. */
		    float response = read_imagef(surface, sampler,
		                                 (float2)(wavelength,
		                                          tri.mat)).x * 0.8f;

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
		}            
        
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

    return;

    #if 0

    /* Intersect ray with triangles. */
    float min_t = INFINITY;
    int closest = -1;
    
    for (int i = 0; i < 3; ++i)
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

    /* Output a color if intersected. */
    if (closest == -1) render[get_global_id(0)] = (float4)(0, 0, 0, 1);
    if (closest == 0)  render[get_global_id(0)] = (float4)(0, 1, 0, 1);
    if (closest == 1)  render[get_global_id(0)] = (float4)(u1, 0, 0, 1);
    if (closest == 2)  render[get_global_id(0)] = (float4)(0, 0, u2, 1);
    return;

    /* Output some stuff. */
    render[get_global_id(0)] = (float4)((direction.x + 1) * 0.5,
                                        (direction.y + 1) * 0.5,
                                        (direction.z + 1) * 0.5, 1);

    #endif

	// render[get_global_id(0)] = (float4)(y, 0, x, 1);

	#if 0

    /* Get pixel position, etc... */
    float3 origin = ...;
    float3 direction = ...;

    /* Prepare the material stack. */
    int matStack[16];
    int matCount = 0;
    
    /* Path tracing. */
    while (true)
    {
        /* Find intersection distance to the nearest triangle. */
        float t;
        if (!Intersection(origin, direction, &t, ...))
        {
            /* No intersection, just return black. */
            ... 0.0f 
            return;
        }

		/* Read the material properties of the material we're currently in. */
		float2 matIndex = (float2)(lambda, matStack[matCount]);
		float4 mt1 = read_imagef(materials, sampler, matIndex);
        
        /* Otherwise, use probability distribution ke^tk to decide whether the
         * ray will make it to that triangle without getting scattered. */
        // do stuff here...
        if (makes it)
        {
            /* First, check if the intersected triangle is a light source, if
             * it is, we are done and just return that. */
            if (is light source)
            {
                ... radiantEmittance(lambda);
                return;
            }

            /* Use fresnel equations (using material settings) to decide if
             * ray should be refracted or reflected, handle interference and
             * update material stack in case of refraction. */
            fresnel equations here

			/* Layer resolution here. */

			if (transmitted)
			{
				/* Did we intersect the material we were already in? */
			}
			else
			{
				/* Reflected, we're done. */
			}
        }
        else
        {
            /* Scatter the ray, using the material's phase function. Do not
             * update the material stack as there is no medium change. */
            phase function scattering here

			/* Go over each light and direct-sample it with an intersection test,
			 * taking into account the phase function's PDF. */
        }

        /* Russian roulette here. Note radiance is subcritical. */
        if (u > radiance)
        {
            /* Return zero. */
            ... 0.0f
            return;
        }

        /* Otherwise, go for one more bounce. */
    }
	#endif
}
