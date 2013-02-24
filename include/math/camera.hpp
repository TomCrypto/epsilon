#pragma once

#include <engine/architecture.hpp>
#include <math/vector.hpp>

/** @file camera.hpp
  * @brief Camera manipulation.
**/

/** @class Camera
  * @brief Device-side camera.
  *
  * This is a simple implementation of a camera, with position/orientation,
  * field of view. Uses linear algebra to project a focal plane.
  *
  * This kernel object does not handle any queries.
**/
class Camera : public KernelObject
{
    private:
        cl::Buffer buffer;

    public:
        Camera(EngineParams& params);
        ~Camera() { }

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};
