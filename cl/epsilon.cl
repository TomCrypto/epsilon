#include <prng.cl>

typedef struct __attribute__ ((packed)) Params
{
	uint width, height;
} Params;

void kernel clmain(global float4 *render, /* Render buffer in XYZn format. */
				   constant Params *params, /* Scene information. */
				   constant ulong4 *seed /* Seed for the PRNG. */
                   )
{
	/* Get a PRNG instance for this worker. */
    PRNG prng = init(get_global_id(0), seed);

	float x = (float)(get_global_id(0) % params->width) / params->width;
	float y = (float)(get_global_id(0) / params->width) / params->height;
    render[get_global_id(0)] += (float4)(rand(&prng) * y, rand(&prng) * x, 0, 1);
}
