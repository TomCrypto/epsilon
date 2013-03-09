#pragma once

#include <engine/architecture.hpp>

/** @file geometry.hpp
  * @brief Geometry handling.
**/

/** @class Geometry
  * @brief Scene-wide geometry.
  *
  * This kernel object manages the list of triangles in the scene to render.
  * 
  * This kernel object handles the following queries:
  * - \c Query::TriangleCount
**/
class Geometry : public KernelObject
{
    private:
        /** @brief Contains the list of triangles in the scene. **/
        cl::Buffer triangles;

        /** @brief Contains the number of triangles in the scene. **/
        uint32_t count;

        /** @brief Describes the BVH leaf size (performance parameter). **/
        uint32_t leafSize;

        /** @brief Contains the BVH nodes (for traversal). **/
        cl::Buffer nodes;

    public:
        Geometry(EngineParams& params);
        ~Geometry();

        void Bind(cl_uint* index);
        void Update(size_t pass);
        void* Query(size_t query);
};
