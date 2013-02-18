#pragma once

/* This file implements a camera manager, which allows perspective projection *
 * of the scene, and includes features such as field of view, depth of field, *
 * etc... Most of the computations are done host-side and sent to the device. */

#include <common/common.hpp>

/* This is a device-side camera structure. */
struct cl_camera
{
    cl_float4 p[4]; /* The focal plane.     */
    cl_float4 pos;  /* The camera position. */
};

struct Camera
{
    private:
        Vector focalPlane[4];
        Vector cameraPos;
        cl_mem buffer;

    public:
        Camera(const Vector& pos, const Vector& dir, const float FOV,
               cl_context context, cl_command_queue queue);

        void Bind(cl_kernel kernel, cl_uint index);
        ~Camera();
        void CL(cl_camera *out);
};
