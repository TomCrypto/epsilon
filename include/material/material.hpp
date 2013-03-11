#pragma once

#include <engine/architecture.hpp>

/** @file material.hpp
  * @brief Material handling.
**/

/** @class Materials
  * @brief Device-side materials.
  *
  * This kernel object is responsible for mapping material ID's to device-side
  * materials, via a mapping function. This is used to decouple geometry and
  * material system.
  *
  * This kernel object handles no queries.
**/
class Materials : public KernelObject
{
    private:
        /** @brief This is the material mapping. **/
        cl::Buffer mapping;

    public:
        Materials(EngineParams& params);
        ~Materials() { }

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};
