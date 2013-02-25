#pragma once

#include <engine/architecture.hpp>
#include <math/aabb.hpp>

/** @file geometry.hpp
  * @brief Geometry handling.
**/

/* This is the OpenCL triangle structure which will be sent to the device. It *
 * does not contain as much information as the host-side implementation since *
 * things like the triangle's centroid, or bounding box are not necessary for *
 * rendering, but only for building the bounding volume hierarchy, host-side. */



struct cl_triangle
{
    cl_float4 p; /* The main triangle vertex. */
    cl_float4 x; /* The "left" triangle edge. */
    cl_float4 y; /* The other triangle edge.  */
    cl_float4 n; /* The triangle's normal.    */
	cl_float mat;
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
    public:
        /* The vertices... */
        Vector p1, p2, p3;
		float material;

        /* Precomputed... */
        Vector x, y, n, c;
		AABB boundingBox;
		Vector centroid;

        /* Creates the triangle from three points. */
        Triangle(Vector p1, Vector p2, Vector p3, float material);

		/* This function returns the bounding box of the triangle. */
        AABB BoundingBox() { return this->boundingBox; }

        /* This function returns the centroid of the triangle. */
        Vector Centroid() { return this->centroid; }

        /* Outputs the equivalent OpenCL triangle structure. */
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
  * This kernel object is passive, and handles the following queries:
  * - \c Query::TriangleCount
**/
class Geometry : public KernelObject
{
    private:
		void BuildBVH();

        std::vector<Triangle*> list;
        cl::Buffer triangles;
        uint32_t count;

		cl::Buffer sceneInfo;

		uint32_t leafSize;
		uint32_t nLeafs, nNodes;
		BVHFlatNode* flatTree;

		cl::Buffer nodes;

    public:
        Geometry(EngineParams& params);
        ~Geometry();

        void Bind(cl_uint* index);
        void Update(size_t pass);
        void* Query(size_t query);
};
