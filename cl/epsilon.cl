#include <prng.cl>

void kernel clmain(global float4 *render, /* Render buffer in XYZn format. */
				   constant ulong4 *seed /* Seed for the PRNG. */
                   )
{
	/* Get a PRNG instance for this worker. */
    PRNG prng = init(get_global_id(0), seed);

    render[get_global_id(0)] = (float4)(rand(&prng), rand(&prng), rand(&prng), 1);
}
