#include <math/prng.hpp>

/* Device-side representation. */
struct cl_prng { cl_ulong4 seed; };

PRNG::PRNG(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <PRNG>...");

    cl_int error;
    this->buffer = cl::Buffer(params.context, CL_MEM_READ_ONLY,
                              sizeof(cl_prng), nullptr, &error);
    Error::Check(Error::Memory, error);

    this->seed = 0;

    fprintf(stderr, " complete.\n\n");
}

void PRNG::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@PRNG> to index %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg((*index)++, buffer));
}

void PRNG::Update(size_t /* index */)
{
    cl_int error = params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                                   sizeof(uint64_t),
                                                   &this->seed);
    Error::Check(Error::CLIO, error);
    this->seed++;
}

void* PRNG::Query(size_t /* query */)
{
    return nullptr;
}
