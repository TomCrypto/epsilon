#pragma once

#include <common/common.hpp>
#include <engine/architecture.hpp>

/** @file epsilon.hpp
  * @brief Epsilon rendering engine.
  *
  * Thie file contains the renderer.
***/

/** @class Epsilon
  * @brief The renderer.
  *
  *
**/
class Epsilon
{
    private:
        /** @brief List of kernel objects used by the renderer. **/
        std::vector<KernelObject*> objects;
        /** @brief The engine parameters. **/
        EngineParams params;

    public:
        /** @brief Initializes the renderer.
          * @param width The render width, in pixels.
          * @param height The render height, in pixels.
          * @param samples The number of samples per pixel.
          * @param platform The OpenCL platform to use for rendering.
          * @param device The OpenCL device to use for rendering.
          * @param source The engine source (scene directory).
          * @param output The engine output (PPM image file).
        **/
        Epsilon(size_t width, size_t height, size_t samples,
                cl::Platform platform, cl::Device device,
                std::string source, std::string output);
};
