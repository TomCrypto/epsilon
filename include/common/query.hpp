#pragma once

#include <common/common.hpp>

/** @file query.hpp
  * @brief Contains kernel object query definitions.
**/

/** @namespace Query
  * @brief Kernel object query definitions.
**/
namespace Query
{
    /** @brief Queries the renderer's overall progress.
      * @note \c Query will return a \c double between 0 and 1 inclusive.
    **/
    extern const size_t Progress;

    /** @brief Queries the number of triangles in the scene.
      * @note \c Query will return a \c uint32_t.
    **/
    extern const size_t TriangleCount;

    /** @brief Queries the estimated time to completion.
      * @note \c Query will return a \c double representing the estimated
      *       number of seconds until the renderer finishes.
      * @note If the value returned is negative, it means time to completion
      *       is not yet available and is indeterminate.
    **/
    extern const size_t EstimatedTime;

	/** @brief Queries the time elapsed since rendering started.
      * @note \c Query will return a \c double representing the number of
              seconds elapsed since the renderer started working.
    **/
    extern const size_t ElapsedTime;
}
