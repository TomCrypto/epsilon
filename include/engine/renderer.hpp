#pragma once

#include <engine/architecture.hpp>

#include <math/prng.hpp>
#include <render/render.hpp>
#include <misc/misc.hpp>
#include <math/camera.hpp>
#include <geometry/geometry.hpp>

/** @file renderer.hpp
  * @brief ɛpsilon rendering engine.
  *
  * This file contains the renderer.
  *
  * \todo Clean up geometry (BVH building, specifically).
  * \todo When all of the above works perfectly, implement material system &
  *       optimize/document the kernel code.
  *
  * \todo [Minor annoyance] Find a portable way to print size_t's.
  * \todo [Low priority] Comment the source files.
  *
**/

/** @class Renderer
  * @brief Engine implementation.
  *
  *
**/
class Renderer
{
    private:
        /** @brief List of kernel objects used by the renderer. **/
        std::vector<KernelObject*> objects;
        /** @brief The engine parameters. **/
        EngineParams params;
        /** @brief The current render pass. **/
        size_t currentPass;

    public:
        /** @brief Initializes the renderer.
          * @param width The render width, in pixels.
          * @param height The render height, in pixels.
          * @param passes The number of render passes (per pixel).
          * @param platform The OpenCL platform to use for rendering.
          * @param device The OpenCL device to use for rendering.
          * @param source The engine source (scene directory).
          * @param output The engine output (PPM image file).
        **/
        Renderer(size_t width, size_t height, size_t passes,
                cl::Platform platform, cl::Device device,
                std::string source, std::string output);

        /** @brief Frees the renderer and all associated resources.
        **/
        ~Renderer();

        /** @brief Instructs the renderer to perform one pass.
          * @returns Returns \c true if this was the final pass, and
          *          \c false otherwise.
        **/  
        bool Execute();

        /** @brief Globally queries all kernel objects.
          * @param query The information to query.
        **/
        void* Query(size_t query);
};
