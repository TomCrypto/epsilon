#include <geometry/geometry.hpp>

/* This will create a triangle from three distinct vertices, and produce some *
 * precomputed values for use in other rendering subsystems.                  */
Triangle::Triangle(Vector p1, Vector p2, Vector p3, float material)
: p1(p1), p2(p2), p3(p3)
{
    /* Compute triangle edges... */
    this->x = this->p2 - this->p1;
    this->y = this->p3 - this->p1;

    /* Compute unsigned normal from the edges. */
    this->n = normalize(cross(this->x, this->y));

    /* Get the triangle centroid (center of gravity). */
    this->c = (this->p1 + this->p2 + this->p3) / 3.0f;

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

	this->material = material;
}

/* This will format the triangle for export to the OpenCL device, with enough *
 * information to enable rendering, i.e. ray-triangle intersection as well as *
 * the triangle's surface normal. Nothing else is required for rendering.     */
void Triangle::CL(cl_triangle *out)
{
    this->p1.CL(&out->p);
    this->x.CL(&out->x);
    this->y.CL(&out->y);
    this->n.CL(&out->n);
	out->mat = this->material;
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

    /* READ GEOMETRY DATA HERE. */

	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(+5, -5, -20), Vector(+5, -5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(+5, -5, +5), Vector(-5, -5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(-5, -5, +5), Vector(-5, +5, +5), 440));
	this->list.push_back(new Triangle(Vector(-5, -5, -20), Vector(-5, +5, +5), Vector(-5, +5, -20), 440));
	this->list.push_back(new Triangle(Vector(+5, -5, -20), Vector(+5, -5, +5), Vector(+5, +5, +5), 530));
	this->list.push_back(new Triangle(Vector(+5, -5, -20), Vector(+5, +5, +5), Vector(+5, +5, -20), 530));
	this->list.push_back(new Triangle(Vector(-5, -5, +5), Vector(+5, -5, +5), Vector(+5, +5, +5), 0));
	this->list.push_back(new Triangle(Vector(-5, -5, +5), Vector(+5, +5, +5), Vector(-5, +5, +5), 0));

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

			Vector p1 = Vector(triangle.p1[0], triangle.p1[1], triangle.p1[2] - 3);
			Vector p2 = Vector(triangle.p2[0], triangle.p2[1], triangle.p2[2] - 3);
			Vector p3 = Vector(triangle.p3[0], triangle.p3[1], triangle.p3[2] - 3);

			//if (triangle.material == 3) this->list.push_back(new Triangle(p1, p2, p3, 660));
			if (triangle.material == 4) this->list.push_back(new Triangle(p1, p2, p3, 580));
		}
    }

	file.close();
    cl_int error;

	fprintf(stderr, "There are %u triangles.\n", (uint32_t)this->list.size());

    /* END READ DATA. */

	fprintf(stderr, "Now building BVH...\n");

	/* BVH INITIALIZATION HERE. */
	this->leafSize = 2;
	this->nNodes = 0;
	this->nLeafs = 0;
	this->flatTree = nullptr;
	BuildBVH();

	fprintf(stderr, "BVH built!\n");
	fprintf(stderr, "Now compacting BVH (%d nodes)...\n", nNodes);

	cl_node* rawNodes = new cl_node[this->nNodes];
	for (size_t t = 0; t < this->nNodes; ++t)
	{
		flatTree[t].bbox.min.CL(&rawNodes[t].bbox_min);
		flatTree[t].bbox.max.CL(&rawNodes[t].bbox_max);
		rawNodes[t].data.s[0] = flatTree[t].start;
		rawNodes[t].data.s[1] = flatTree[t].nPrims;
		rawNodes[t].data.s[2] = flatTree[t].rightOffset;
		rawNodes[t].data.s[3] = 0;
	}

	fprintf(stderr, "BVH compacted, uploading...\n");

	this->nodes = cl::Buffer(params.context,
							 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
						     sizeof(cl_node) * nNodes, rawNodes, &error);
	Error::Check(Error::Memory, error);

	fprintf(stderr, "BVH uploaded! (now destroying the compact array)\n");
	delete[] rawNodes;

	fprintf(stderr, "Compacting triangles...\n");

	this->count = this->list.size();
    cl_triangle* raw = new cl_triangle[this->count];
    for (size_t t = 0; t < this->count; ++t) list[t]->CL(raw + t);

	fprintf(stderr, "Triangles compacted, uploading...\n");

    this->triangles = cl::Buffer(params.context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(cl_triangle) * this->count,
                                 raw, &error);
    Error::Check(Error::Memory, error);

	fprintf(stderr, "Triangle data uploaded! Deleting...\n");
	delete [] raw;

	cl_info info;
	info.triangleCount = this->count;
	info.nodeCount = this->nNodes;

	this->sceneInfo = cl::Buffer(params.context,
						     	 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
								 sizeof(cl_info), &info, &error);
	Error::Check(Error::Memory, error);

	fprintf(stderr, "Uploaded scene data!\n");
    fprintf(stderr, "Initialization complete.\n\n");
}

Geometry::~Geometry()
{
	for (size_t t = 0; t < list.size(); ++t) delete list[t];
}

void Geometry::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <triangles@Geometry> to index %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->triangles));
    (*index)++;
    fprintf(stderr, "Binding <nodes@Geometry> to index %u.\n", *index);
	Error::Check(Error::Bind, params.kernel.setArg(*index, this->nodes));
    (*index)++;
    fprintf(stderr, "Binding <sceneInfo@Geometry> to index %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->sceneInfo));
    (*index)++;
}

void Geometry::Update(size_t /* index */)
{
    return;
}

void* Geometry::Query(size_t query)
{
    if (query == Query::TriangleCount) return &this->count;
    return nullptr;
}

struct BVHBuildEntry {
 // If non-zero then this is the index of the parent. (used in offsets)
 uint32_t parent;
 // The range of objects in the object list covered by this node.
 uint32_t start, end;
};

void Geometry::BuildBVH()
{
 BVHBuildEntry todo[128];
 uint32_t stackptr = 0;
	const uint32_t Untouched    = 0xffffffff;
	const uint32_t TouchedTwice = 0xfffffffd;

 // Push the root
 todo[stackptr].start = 0;
 todo[stackptr].end = this->list.size();
 todo[stackptr].parent = 0xfffffffc;
 stackptr++;

	BVHFlatNode node;
	std::vector<BVHFlatNode> buildnodes;
	buildnodes.reserve(this->list.size()*2);

 while(stackptr > 0) {
		// Pop the next item off of the stack
		BVHBuildEntry &bnode( todo[--stackptr] );
		uint32_t start = bnode.start;
		uint32_t end = bnode.end;
		uint32_t nPrims = end - start;

		nNodes++;
		node.start = start;
		node.nPrims = nPrims;
		node.rightOffset = Untouched;

		// Calculate the bounding box for this node
		AABB bb( this->list[start]->BoundingBox());
		AABB bc( this->list[start]->Centroid());
		for(uint32_t p = start+1; p < end; ++p) {
			bb.ExpandToInclude( this->list[p]->BoundingBox());
			bc.ExpandToInclude( this->list[p]->Centroid());
		}
		node.bbox = bb;

  // If the number of primitives at this point is less than the leaf
  // size, then this will become a leaf. (Signified by rightOffset == 0)
		if(nPrims <= leafSize) {
			node.rightOffset = 0;
			nLeafs++;
		}

		buildnodes.push_back(node);

		// Child touches parent...
		// Special case: Don't do this for the root.
		if(bnode.parent != 0xfffffffc) {
			buildnodes[bnode.parent].rightOffset --;

			// When this is the second touch, this is the right child.
			// The right child sets up the offset for the flat tree.
			if( buildnodes[bnode.parent].rightOffset == TouchedTwice ) {
				buildnodes[bnode.parent].rightOffset = nNodes - 1 - bnode.parent;
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
			if( this->list[i]->Centroid()[split_dim] < split_coord ) {
				std::swap( this->list[i], this->list[mid] );
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
		todo[stackptr].parent = nNodes-1;
		stackptr++;

		// Push left child
		todo[stackptr].start = start;
		todo[stackptr].end = mid;
		todo[stackptr].parent = nNodes-1;
		stackptr++;
 }

	// Copy the temp node data to a flat array
	flatTree = new BVHFlatNode[nNodes];
	for(uint32_t n=0; n<nNodes; ++n)
		flatTree[n] = buildnodes[n];
}
