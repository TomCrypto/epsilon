#include <math/prng.hpp>

PRNG::PRNG(cl_context context, cl_command_queue queue)
{
    /* Initialize seed to zero. */
    this->seed = 0;
    cl_int error = 0;

    /* Create the PRNG buffer. */
    this->buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_prng),
                                  0, 0);

    if (this->buffer == nullptr) throw std::runtime_error(Error(E_BUF, error));
}

void PRNG::Bind(cl_kernel kernel, cl_uint index)
{
    cl_int error = clSetKernelArg(kernel, index, sizeof(this->buffer),
                                    &this->buffer);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_BIND, error));
}

void PRNG::Renew(cl_command_queue queue)
{
    /* Increment seed and upload new value. */
    this->seed++;

    cl_int error = clEnqueueWriteBuffer(queue, this->buffer, CL_TRUE, 0,
                                    sizeof(uint64_t), &this->seed, 0, 0, 0);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_WRITE, error));
}

PRNG::~PRNG()
{
    clReleaseMemObject(this->buffer);
}
