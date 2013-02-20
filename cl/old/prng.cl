/* This macro will convert a 64-bit integer to a [0..1) float. */
#define TO_FLOAT(x) ((float)x / (ulong)(18446744073709551615UL))

/* This indicates the one-way function quality. Recommend at least four. */
#define ROUNDS 9

/* This is a 512-bit > 256-bit one-way function. */
void renew(ulong4 *output, constant ulong4 *seed)
{
    /* Retain the initial input... */
    ulong4 block = *output + *seed;

    #pragma unroll
    for (ulong t = 0; t < ROUNDS; t++)
    {
        /* Round 2t + 0 (×4 mix & permutation). */
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(14, 16));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(52, 57));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(23, 40));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)( 5, 37));
        block.hi ^= block.lo; block = block.xywz;

        /* Key addition. */
        block += *seed;

        /* Round 2t + 1 (×4 mix & permutation). */
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(25, 33));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(46, 12));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(58, 22));
        block.hi ^= block.lo; block = block.xywz;
        block.lo += block.hi; block.hi = rotate(block.hi, (ulong2)(32, 32));
        block.hi ^= block.lo; block = block.xywz;

        /* Key addition. */
        block += *seed;
    }

    /* Feed-forward. */
    *output ^= block;
}

/* This PRNG structure, contains an intelligent pseudorandom number generator *
 * which efficiently generates uniform floating-point numbers.                */
typedef struct PRNG
{
	ulong4 state;          /* The internal state.  */
	uint pointer;          /* The usage pointer.   */
    constant ulong4 *seed; /* Pointer to the seed. */
} PRNG;

/* This function will initialize a new PRNG state, based on an ID, which must *
 * be unique across all work items (otherwise the PRNG's will output the same *
 * pseudorandom numbers). Usually, get_global_id works fine for this.         */
PRNG init(ulong ID, constant ulong4 *seed)
{
    PRNG instance;
    instance.state = (ulong4)(ID);
    instance.pointer = 0;
    instance.seed = seed;
    return instance;
}

/* This function will return a uniform pseudorandom number in [0..1). */
float rand(PRNG *prng)
{
    /* Do we need to renew? */
    if (prng->pointer == 0)
    {
         renew(&prng->state, prng->seed);
         prng->pointer = 4;
    }

    /* Return a uniform number in the desired interval. */
    --prng->pointer;
    if (prng->pointer == 3) return TO_FLOAT(prng->state.w);
    if (prng->pointer == 2) return TO_FLOAT(prng->state.z);
    if (prng->pointer == 1) return TO_FLOAT(prng->state.y);
    return TO_FLOAT(prng->state.x);
}
