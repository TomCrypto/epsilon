#include <prng.cl>

void kernel clmain(constant ulong4 *seed /* Seed for the PRNG. */
                   )
{
	/* Get a PRNG instance for this worker. */
    PRNG prng = init(get_global_id(0), seed);

    /* Get pixel coordinates in normalized coordinates [0..1). */
    float a1 = rand(&prng) - 0.5f;
    float a2 = rand(&prng) - 0.5f;
}
