#pragma once

#include <common/common.hpp>
#include <engine/architecture.hpp>

/** @file misc.hpp
  * @brief Miscellaneous engine tools
**/

/** @class Progress
  * @brief Time-based renderer metrics

  * This kernel object does not actually bind anything to the device but simply
  * provides metrics on the renderer's current progress, and calculates the ETC
  * (estimated time to completion) of the renderer.
  *
  * It is by definition an active kernel object.
  *
  * This kernel object handles the following queries:
  * - \c Query::Progress
  * - \c Query::EstimatedTime
**/
class Progress : public KernelObject
{
    private:
        time_t startTime;
        double progress;
        double ETC;

    public:
		Progress(const EngineParams& params) : KernelObject(params) { }
		~Progress() { }

        bool IsActive();
        void Initialize();
        void Bind(cl::Kernel kernel, cl_uint slot);
        void Update(size_t index);
        void* Query(size_t query);
};
