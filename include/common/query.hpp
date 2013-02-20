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
}
