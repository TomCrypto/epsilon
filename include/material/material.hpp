#pragma once

/* This contains material definitions, both for surfaces and volumes. */

#include <common/common.hpp>

/* This is a surface material definition. */
struct Surface
{
    /* Just a spectral response for diffuse BRDF (temporary). */
    float diffuse[128];
};

class Materials
{
    private:
        cl_mem surfaces;
    public:
        /* Create a list of N materials. */
        Materials(cl_context context, cl_command_queue queue,
                  Surface *surfaces, size_t surfaceCount);

        /* Bind materials to kernel. */
        void Bind(cl_kernel kernel, cl_uint index);

};
