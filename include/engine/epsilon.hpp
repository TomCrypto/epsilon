#pragma once

/* This file provides the Epsilon engine interface. The engine is initialized *
 * by passing it an OpenCL device, and is subsequently configured.            */

#include <common/common.hpp>

class Epsilon
{
    private:
        cl_device_id device;
        cl_context context;
        cl_command_queue queue;
        size_t localGroupSize;
        cl_program program;
        cl_kernel kernel;

    public:
        Epsilon(cl_device_id device);
};
