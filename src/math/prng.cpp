#include <math/prng.hpp>

/* Device-side representation. */
struct cl_prng { cl_ulong4 seed; };

PRNG::PRNG(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <PRNG>...");

    this->buffer = CreateBuffer(params.context, CL_MEM_READ_ONLY,
                                sizeof(cl_prng), nullptr);

    this->seed = 0;

    fprintf(stderr, " complete.\n\n");
}

void PRNG::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@PRNG> to index %u.\n", *index);
    BindArgument(params.kernel, buffer, (*index)++);
}

void PRNG::Update(size_t /* index */)
{
    WriteToBuffer(params.queue, this->buffer, CL_TRUE,
                  0, sizeof(uint64_t), &this->seed);
    this->seed++;
}

void* PRNG::Query(size_t /* query */)
{
    return nullptr;
}
