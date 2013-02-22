#pragma once

/* This file implements a camera manager, which allows perspective projection *
 * of the scene, and includes features such as field of view, depth of field, *
 * etc... Most of the computations are done host-side and sent to the device. */

#include <common/common.hpp>
#include <engine/architecture.hpp>

/** @file camera.hpp
  * @brief Camera manipulation
**/

/** @class Camera
  * @brief Device-side camera
  *
  * This is a simple implementation of a camera.
  *
  * This kernel object does not handle any queries.
**/
class Camera : public KernelObject
{
    private:
        cl::Buffer buffer;

    public:
        Camera(EngineParams& params) : KernelObject(params) { }
        ~Camera() { }

        bool IsActive();
        void Initialize();
        void Bind(cl_uint* slot);
        void Update(size_t index);
        void* Query(size_t query);
};
