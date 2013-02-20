#pragma once

#include <common/common.hpp>
#include <engine/architecture.hpp>

#include <math/prng.hpp>
#include <render/render.hpp>
#include <misc/etc.hpp>

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
        /** @brief The current sample being rendered. **/
        size_t sampleIndex;

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

        /** @brief Frees the renderer and all associated resources.
        **/
        ~Epsilon();

        /** @brief Executes the renderer for one sample.
        **/  
        void Execute();

        /** @brief Returns whether the render is finished.
        **/
        bool Finished();

        /** @brief Globally queries all kernel objects.
          * @param query The query.
        **/
        void* Query(size_t query);
};
