#pragma once

#include <engine/architecture.hpp>

/** @file misc.hpp
  * @brief Miscellaneous engine tools.
**/

/** @class Progress
  * @brief Time-based renderer metrics.

  * This kernel object does not actually bind anything to the device but simply
  * provides metrics on the renderer's current progress, and calculates the ETC
  * (estimated time to completion) of the renderer.
  *
  * This kernel object handles the following queries:
  * - \c Query::Progress
  * - \c Query::EstimatedTime
  * - \c Query::ElapsedTime
**/
class Progress : public KernelObject
{
    private:
        double startTime;
		double elapsed;
        double progress;
        double remains;

    public:
		Progress(EngineParams& params);
		~Progress() { }

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};
