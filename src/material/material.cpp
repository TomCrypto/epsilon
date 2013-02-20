#include <material/material.hpp>

Materials::Materials(cl_context context, cl_command_queue queue,
                     Surface *surfaces, size_t surfaceCount)
{
    cl_image_format format = { CL_R, CL_FLOAT };
    cl_image_desc desc;
    desc.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
    desc.image_width = 128;
    desc.image_height = 0;
    desc.image_depth = 0;
    desc.image_row_pitch = 0;
    desc.image_slice_pitch = 0;
    desc.num_mip_levels = 0;
    desc.num_samples = 0;
    desc.buffer = 0;
    desc.image_array_size = surfaceCount;

    cl_int error;
    this->surfaces = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc,
                                   nullptr, &error);

	Error::Check(Error::Memory, error);

    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = {128, 1, 1 };

    for (size_t t = 0; t < surfaceCount; ++t)
    {
        origin[1] = t;

        error = clEnqueueWriteImage(queue, this->surfaces, CL_TRUE, origin,
                                    region, 0, 0, surfaces[t].diffuse,
                                    0, 0, 0);

		Error::Check(Error::CLIO, error);
    }
}

void Materials::Bind(cl_kernel kernel, cl_uint index)
{
    cl_int error = clSetKernelArg(kernel, index, sizeof(this->surfaces),
                                  &this->surfaces);

	Error::Check(Error::Bind, error);
}
