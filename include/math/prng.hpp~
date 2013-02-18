#pragma once

/* This manages the device-side PRNG. */

#include <common/common.hpp>

/* Device-side PRNG structure (just the seed, really). */
struct cl_prng
{
    cl_ulong4 seed;
};

struct PRNG
{
    private:
        uint64_t seed;
        cl_mem buffer;

    public:
        PRNG(cl_context context, cl_command_queue queue);
        ~PRNG();

        void Bind(cl_kernel kernel, cl_uint index);
        
        void Renew(cl_command_queue queue);
};
