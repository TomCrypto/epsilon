#pragma once

#include <engine/architecture.hpp>
#include <math/aabb.hpp>

/** @file geometry.hpp
  * @brief Geometry handling.
**/

struct cl_triangle
{
    cl_float4 p; /* The main triangle vertex. */
    cl_float4 x; /* The "left" triangle edge. */
    cl_float4 y; /* The other triangle edge.  */
    cl_float4 n; /* The triangle's normal.    */
    // add tangent/bitangent here as well
	cl_int mat;  /* The triangle's material.  */
};


/** @class Triangle
  * @brief Device-side triangle.
  *
  * This represents a triangle. Note that the actual triangle data sent to the
  * device is a subset of what this class contains, since not all information
  * is useful for rendering, some of it being only needed for initialization.
**/
class Triangle
{
    private:
        Vector p1, p2, p3;
        AABB boundingBox;
        Vector centroid;
        Vector x, y, n;

    public:
        /** @brief The triangle's normalized material index. **/
        uint32_t material;

        /** @brief The triangle's unnormalized material ID (as a string). **/
        std::string rawMaterial;

        /** @brief Creates the triangle from three points (vertices).
          * @param p1 The first vertex.
          * @param p2 The second vertex.
          * @param p3 The third vertex.
          * @param material The triangle's material ID.
        **/
        Triangle(Vector p1, Vector p2, Vector p3, std::string material);

        /** @brief Returns the triangle's (minimum) bounding box.
        **/
        AABB BoundingBox() { return this->boundingBox; }

        /** @brief Returns the triangle's centroid.
        **/
        Vector Centroid() { return this->centroid; }

        /** @brief Converts the triangle to a device-side representation.
          * @param out A pointer to write the output to.
        **/
        void CL(cl_triangle *out);
};

struct BVHFlatNode
{
	AABB bbox;
	uint32_t start, nPrims, rightOffset;
};

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
		void BuildBVH(std::vector<Triangle*>& list, uint32_t leafSize,
                      uint32_t* leafCount, uint32_t* nodeCount,
                      BVHFlatNode** bvhTree);

        /** @brief Contains the list of triangles in the scene. **/
        cl::Buffer triangles;

        /** @brief Contains the number of triangles in the scene. **/
        uint32_t count;

        /** @brief Contains the geometry information (e.g. triangle count). **/
		cl::Buffer info;

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
