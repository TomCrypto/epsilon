#pragma once

/* This file contains the spectral to XYZ color-matching curve and additional *  * utility functions related to color. */

#include <common/common.hpp>

struct XYZCurve
{
    private:
        cl_mem curve;

    public:
        /* Sets up the color matching curve on a given device. */
        XYZCurve(cl_context context, cl_command_queue queue);
        ~XYZCurve();

        /* Binds the color-matching curve to a kernel argument. */
        void Bind(cl_kernel kernel, cl_uint index);
};
