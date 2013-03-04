#include <geometry/geometry.hpp>
#include <misc/xmlutils.hpp>
#include <misc/pugixml.hpp>
#include <math/aabb.hpp>

#include <memory>
#include <set>

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

struct BVHBuildEntry {
 // If non-zero then this is the index of the parent. (used in offsets)
 uint32_t parent;
 // The range of objects in the object list covered by this node.
 uint32_t start, end;
};

void BuildBVH(std::vector<Triangle*>& list, uint32_t leafSize,
              uint32_t* leafCount, uint32_t* nodeCount,
              BVHFlatNode** bvhTree)
{
 BVHBuildEntry todo[128];
 uint32_t stackptr = 0;
	const uint32_t Untouched    = 0xffffffff;
	const uint32_t TouchedTwice = 0xfffffffd;

 // Push the root
 todo[stackptr].start = 0;
 todo[stackptr].end = list.size();
 todo[stackptr].parent = 0xfffffffc;
 stackptr++;

	BVHFlatNode node;
	std::vector<BVHFlatNode> buildnodes;
	buildnodes.reserve(list.size()*2);

 while(stackptr > 0) {
		// Pop the next item off of the stack
		BVHBuildEntry &bnode( todo[--stackptr] );
		uint32_t start = bnode.start;
		uint32_t end = bnode.end;
		uint32_t nPrims = end - start;

		(*nodeCount)++;
		node.start = start;
		node.nPrims = nPrims;
		node.rightOffset = Untouched;

		// Calculate the bounding box for this node
		AABB bb( list[start]->BoundingBox());
		AABB bc( list[start]->Centroid());
		for(uint32_t p = start+1; p < end; ++p) {
			bb.ExpandToInclude( list[p]->BoundingBox());
			bc.ExpandToInclude( list[p]->Centroid());
		}
		node.bbox = bb;

  // If the number of primitives at this point is less than the leaf
  // size, then this will become a leaf. (Signified by rightOffset == 0)
		if(nPrims <= leafSize) {
			node.rightOffset = 0;
		    (*leafCount)++;
		}

		buildnodes.push_back(node);

		// Child touches parent...
		// Special case: Don't do this for the root.
		if(bnode.parent != 0xfffffffc) {
			buildnodes[bnode.parent].rightOffset --;

			// When this is the second touch, this is the right child.
			// The right child sets up the offset for the flat tree.
			if( buildnodes[bnode.parent].rightOffset == TouchedTwice ) {
				buildnodes[bnode.parent].rightOffset = *nodeCount - 1 - bnode.parent;
			}
		}

		// If this is a leaf, no need to subdivide.
		if(node.rightOffset == 0)
			continue;

		// Set the split dimensions
		uint32_t split_dim = bc.Split();

		// Split on the center of the longest axis
		float split_coord = .5f * (bc.min[split_dim] + bc.max[split_dim]);

		// Partition the list of objects on this split
		uint32_t mid = start;
		for(uint32_t i=start;i<end;++i) {
			if( list[i]->Centroid()[split_dim] < split_coord ) {
				std::swap( list[i], list[mid] );
				++mid;
			}
		}

		// If we get a bad split, just choose the center...
		if(mid == start || mid == end) {
			mid = start + (end-start)/2;
		}

		// Push right child
		todo[stackptr].start = mid;
		todo[stackptr].end = end;
		todo[stackptr].parent = (*nodeCount)-1;
		stackptr++;

		// Push left child
		todo[stackptr].start = start;
		todo[stackptr].end = mid;
		todo[stackptr].parent = (*nodeCount)-1;
		stackptr++;
 }

	// Copy the temp node data to a flat array
	*bvhTree = new BVHFlatNode[*nodeCount];
	for(uint32_t n=0; n<*nodeCount; ++n)
		(*bvhTree)[n] = buildnodes[n];
}

/* This will create a triangle from three distinct vertices, and produce some *
 * precomputed values for use in other rendering subsystems.                  */
Triangle::Triangle(Vector p1, Vector p2, Vector p3, std::string material)
: p1(p1), p2(p2), p3(p3)
{
    /* Compute triangle edges... */
    this->x = this->p2 - this->p1;
    this->y = this->p3 - this->p1;

    /* Compute unsigned normal from the edges. */
    this->n = normalize(cross(this->x, this->y));

	/* Compute the triangle's bounding box. */
    Vector lo = Vector(std::min(p1.x, std::min(p2.x, p3.x)),
                       std::min(p1.y, std::min(p2.y, p3.y)),
                       std::min(p1.z, std::min(p2.z, p3.z)));
    Vector hi = Vector(std::max(p1.x, std::max(p2.x, p3.x)),
                       std::max(p1.y, std::max(p2.y, p3.y)),
                       std::max(p1.z, std::max(p2.z, p3.z)));
    this->boundingBox = AABB(lo, hi);

    /* Compute the triangle's centroid. */
    this->centroid = (p1 + p2 + p3) / 3.0f;

	this->rawMaterial = material;
}

/* This will format the triangle for export to the OpenCL device, with enough *
 * information to enable rendering, i.e. ray-triangle intersection as well as *
 * the triangle's surface normal. Nothing else is required for rendering.     */
void Triangle::CL(cl_triangle *out)
{
    out->mat = material;
    p1.CL(&out->p);
    x.CL(&out->x);
    y.CL(&out->y);
    n.CL(&out->n);
}

/******************************************************************************/

/* These are scene entity types, which indicate the nature of the next object in the scene file. */
enum EntityType { COLORSYSTEM = 0, CAMERA = 1, DISTRIBUTION = 2, MATERIAL = 3, LIGHT = 4, PRIMITIVE = 5 };

/* This is an entity header. */
#pragma pack(1)
struct EntityHeader
{
    /* The entity type. */
    EntityType type;
    /* The subtype. */
    uint32_t subtype;
};

/* Utility function to read a scene file entity header from a file. */
bool ReadHeader(std::fstream& file, EntityHeader* header)
{
    file.read((char*)header, sizeof(EntityHeader));
    return (!file.eof());
}

/* This is a triangle compatible with my older raytracer (Lambda) to
 * quickly load geometry from existing scene files. */
struct LambdaTriangle
{
	int32_t material, light;
	float p1[3], p2[3], p3[3];
};

struct cl_info
{
	cl_uint triangleCount;
	cl_uint nodeCount;
};

struct __attribute__ ((packed)) cl_node
{
	cl_float4 bbox_min;
	cl_float4 bbox_max;
	cl_uint4 data; // start || nPrims || rightOffset
};

Geometry::Geometry(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <Geometry>.\n");
    fprintf(stderr, "Loading '*/geometry.xml'.\n");

    std::fstream stream;
    pugi::xml_document doc;
    GetData("geometry.xml", stream);
    fprintf(stderr, "Parsing scene geometry...");

    if (!doc.load(stream)) Error::Check(Error::IO, 0, true);
    fprintf(stderr, " done.\n");

    pugi::xml_node node = doc.child("geometry");
    leafSize = node.child("general").attribute("leaf").as_int();

    std::vector<Triangle*> triangleList;
    std::set<std::string> materialList;

    for (pugi::xml_node tri : node.child("data").children("triangle"))
    {
        Vector p1 = parseVector(tri.child("p1"));
        Vector p2 = parseVector(tri.child("p2"));
        Vector p3 = parseVector(tri.child("p3"));

        std::string material = tri.attribute("ID").value();
        triangleList.push_back(new Triangle(p1, p2, p3, material));
        materialList.insert(material);
        ++count;
    }

    for (size_t t = 0; t < count; ++t)
    {
        size_t index;
        std::string m = triangleList[t]->rawMaterial;
        index = std::distance(materialList.begin(), materialList.find(m));
        triangleList[t]->material = index;
    }

    /* READ GEOMETRY DATA HERE. */

    #if 0

	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(+5, -5, -20), Vector(+5, -5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(+5, -5, +5), Vector(-5, -5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(-5, -5, +5), Vector(-5, +5, +5), 440));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(-5, +5, +5), Vector(-5, +5, -20), 440));
	this->list.push_back(new Triangle(Vector(+5, -5, -20), Vector(+5, -5, +5), Vector(+5, +5, +5), 660));
	this->list.push_back(new Triangle(Vector(+5, -5, -20), Vector(+5, +5, +5), Vector(+5, +5, -20), 660));
	this->list.push_back(new Triangle(Vector(-5, -5, +5), Vector(+5, -5, +5), Vector(+5, +5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, +5), Vector(+5, +5, +5), Vector(-5, +5, +5), 0));

	this->list.push_back(new Triangle(Vector(-5, +5, -20), Vector(+5, +5, -20), Vector(+5, +5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, +5, -20), Vector(+5, +5, +5), Vector(-5, +5, +5), 0));

	this->list.push_back(new Triangle(Vector(-2, +4.95, -2), Vector(+2, +4.95, -2), Vector(+2, +4.95, +2), -1));
	this->list.push_back(new Triangle(Vector(-2, +4.95, -2), Vector(+2, +4.95, +2), Vector(-2, +4.95, +2), -1));

	#endif

	#if 0

    std::fstream file;
    GetData("geometry", file);

	/* Read every scene entity in the file. */
    EntityHeader header;
    while (ReadHeader(file, &header))
    {
        /* Check the entity type to know what to do. */
		if (header.type == PRIMITIVE)
		{
			LambdaTriangle triangle;
			file.read((char*)&triangle, sizeof(LambdaTriangle));

			/* Vector p1 = Vector(triangle.p1[0] + 1.5, triangle.p1[1] + 0.1, triangle.p1[2] - 6);
			Vector p2 = Vector(triangle.p2[0] + 1.5, triangle.p2[1] + 0.1, triangle.p2[2] - 6);
			Vector p3 = Vector(triangle.p3[0] + 1.5, triangle.p3[1] + 0.1, triangle.p3[2] - 6); */

			//if (triangle.material == 3) this->list.push_back(new Triangle(p1, p2, p3, 660));
			if (triangle.material == 3)
			{
				Vector p1 = Vector(triangle.p1[0], triangle.p1[1] + 0.1, triangle.p1[2]);
				Vector p2 = Vector(triangle.p2[0], triangle.p2[1] + 0.1, triangle.p2[2]);
				Vector p3 = Vector(triangle.p3[0], triangle.p3[1] + 0.1, triangle.p3[2]);

				triangleList.push_back(new Triangle(p1, p2, p3, ""));
				triangleList[triangleList.size() - 1]->material = 6;
			}

			/* if (triangle.material == 4)
			{
				Vector p1 = Vector(triangle.p1[0], triangle.p1[1], triangle.p1[2]);
				Vector p2 = Vector(triangle.p2[0], triangle.p2[1], triangle.p2[2]);
				Vector p3 = Vector(triangle.p3[0], triangle.p3[1], triangle.p3[2]);

				triangleList.push_back(new Triangle(p1, p2, p3, ""));
				triangleList[triangleList.size() - 1]->material = 6;
			} */
		}
    }

	file.close();	

	count = triangleList.size();

	#endif

    cl_int error;

	fprintf(stderr, "Total %u triangles.\n", count);
    fprintf(stderr, "Now building BVH.\n");

    uint32_t leafCount = 0, nodeCount = 0;
    BVHFlatNode* bvhTree = nullptr;

    BuildBVH(triangleList, leafSize, &leafCount, &nodeCount, &bvhTree);
    fprintf(stderr, "BVH successfully built, %u nodes.\n", nodeCount);
    fprintf(stderr, "Number of leaves: %u/%u.\n", leafCount, leafSize);
    fprintf(stderr, "Now compacting BVH.\n");

	{	
		const std::unique_ptr<cl_node[]> rawNodes(new cl_node[nodeCount]);
		for (size_t t = 0; t < nodeCount; ++t)
		{
			bvhTree[t].bbox.min.CL(&rawNodes[t].bbox_min);
			bvhTree[t].bbox.max.CL(&rawNodes[t].bbox_max);
			rawNodes[t].data.s[0] = bvhTree[t].start;
			rawNodes[t].data.s[1] = bvhTree[t].nPrims;
			rawNodes[t].data.s[2] = bvhTree[t].rightOffset;
			rawNodes[t].data.s[3] = 0;
		}

		fprintf(stderr, "BVH compacted, uploading to device...\n");

		this->nodes = cl::Buffer(params.context,
								 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								 sizeof(cl_node) * nodeCount, rawNodes.get(),
                                 &error);
		Error::Check(Error::Memory, error);

		fprintf(stderr, "BVH uploaded! Freeing resources.\n");
	}

    delete[] bvhTree;

	fprintf(stderr, "Compacting triangle list.\n");

    cl_triangle* raw = new cl_triangle[count];
    for (size_t t = 0; t < count; ++t) triangleList[t]->CL(raw + t);

	fprintf(stderr, "Triangles compacted, uploading to device...\n");

    this->triangles = cl::Buffer(params.context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(cl_triangle) * count,
                                 raw, &error);
    Error::Check(Error::Memory, error);

	fprintf(stderr, "Triangle data uploaded! Freeing resources.\n");
    for (size_t t = 0; t < count; ++t) delete triangleList[t];
	delete [] raw;

    fprintf(stderr, "Uploading geometry information.\n");

	cl_info geometryInfo;
	geometryInfo.triangleCount = count;
	geometryInfo.nodeCount = nodeCount;

	info = cl::Buffer(params.context,
						     	 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								 sizeof(cl_info), &geometryInfo, &error);
	Error::Check(Error::Memory, error);

    fprintf(stderr, "Initialization complete.\n\n");
    stream.close();
}

Geometry::~Geometry()
{
	//for (size_t t = 0; t < list.size(); ++t) delete list[t];
}

void Geometry::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <triangles@Geometry> to index %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->triangles));
    (*index)++;
    fprintf(stderr, "Binding <nodes@Geometry> to index %u.\n", *index);
	Error::Check(Error::Bind, params.kernel.setArg(*index, this->nodes));
    (*index)++;
    fprintf(stderr, "Binding <info@Geometry> to index %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->info));
    (*index)++;
}

void Geometry::Update(size_t /* index */) { return; }

void* Geometry::Query(size_t query)
{
    return (query == Query::TriangleCount) ? &this->count : nullptr;
}
