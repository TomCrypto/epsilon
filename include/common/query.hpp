#pragma once

#include <common/common.hpp>

/** @file query.hpp
  * @brief Contains kernel object query definitions.
**/

/** @namespace Query
  * @brief This namespace contains kernel object query definitions.
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
      * @note \c Query will return a \c size_t representing the estimated
      *       number of seconds until the renderer finishes.
    **/
    extern const size_t EstimatedTime;
}
