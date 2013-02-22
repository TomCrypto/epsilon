#include <math/prng.hpp>

/* Device-side representation. */
struct cl_prng { cl_ulong4 seed; };

bool PRNG::IsActive() { return true; }

void PRNG::Initialize()
{
    cl_int error;
    this->buffer = cl::Buffer(params.context, CL_MEM_READ_ONLY,
                              sizeof(cl_prng), nullptr, &error);
    Error::Check(Error::Memory, error);

    this->seed = 0;
}

void PRNG::Bind(cl_uint* slot)
{
    Error::Check(Error::Bind, params.kernel.setArg(*slot, this->buffer));
	(*slot)++;
}

void PRNG::Update(size_t index)
{
    cl_int error = params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                                   sizeof(uint64_t),
                                                   &this->seed);
    Error::Check(Error::CLIO, error);
    this->seed++;
}

void* PRNG::Query(size_t query)
{
    return nullptr;
}
