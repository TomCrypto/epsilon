#pragma once

#include <engine/architecture.hpp>
#include <render/spectral.hpp>
#include <math/vector.hpp>

/** @file render.hpp
  * @brief Color-related kernel objects.
  *
  * This file contains kernel objects responsible for handling colors in
  * general.
**/

/** @class PixelBuffer
  * @brief Device-side pixel buffer.
  *
  * This kernel object is responsible for managing a pixel buffer, which is a
  * raster array of pixels which the kernel can read from and write to. This
  * kernel object is actually responsible for saving the final render to
  * the output file, and does so upon destruction.
  *
  * This kernel object handles no queries.
**/
class PixelBuffer : public KernelObject
{
    private:
        cl::Buffer rp;
        cl::Buffer pb;
        float *pixels;

        void Acquire(const EngineParams& params);
        void Upload(const EngineParams& params);

        void WriteToFile(std::string path);
    public:
        PixelBuffer(EngineParams& params);
        ~PixelBuffer();

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};


/** @class Tristimulus
  * @brief Color-matching curve.
  *
  * This kernel object just uploads a color-matching curve to the device, to
  * map optical wavelengths to their tristimulus "perceptual" XYZ values.
  *
  * This kernel object handles no queries.
**/
class Tristimulus : public KernelObject
{
    private:
        cl::Image2D buffer;
    public:
        Tristimulus(EngineParams& params);
        ~Tristimulus() { }

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};
