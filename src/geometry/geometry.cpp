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
    cl_float4 t; /* The triangle's tangent.   */
    cl_float4 b; /* The triangle's bitangent. */
    cl_float4 n; /* The triangle's normal.    */
    cl_uint mat; /* The triangle's material.  */
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
        Vector t, b;

    public:
        /** @brief The triangle's material ID. **/
        uint32_t material;

        /** @brief The triangle's model ID. **/
        std::string model;

        /** @brief Creates the triangle from three points (vertices).
          * @param p1 The first vertex.
          * @param p2 The second vertex.
          * @param p3 The third vertex.
          * @param modelID The triangle's model ID.
        **/
        Triangle(Vector p1, Vector p2, Vector p3, std::string modelID);

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
Triangle::Triangle(Vector p1, Vector p2, Vector p3, std::string modelID)
: p1(p1), p2(p2), p3(p3)
{
    /* Compute triangle edges... */
    this->x = this->p2 - this->p1;
    this->y = this->p3 - this->p1;

    /* Compute unsigned normal from the edges. */
    this->n = normalize(cross(this->x, this->y));

    /* Compute the triangle's TBN matrix. */
    this->t = normalize(this->x);
    this->b = normalize(cross(this->t, this->n));

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

    this->model = modelID;
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
    t.CL(&out->t);
    b.CL(&out->b);
    n.CL(&out->n);
}

/******************************************************************************/

struct cl_node
{
    cl_float4 bbox_min;
    cl_float4 bbox_max;
    cl_uint4 data; // start || nPrims || rightOffset
};

/* Contains model information. */
struct ModelInfo
{
    std::string modelID;
    Vector translation;
    Vector rotation;
    Vector scaling;
};

/* Parses an obj model and returns a list of triangles. */
std::vector<Triangle*> ParseModel(std::fstream &obj, ModelInfo info)
{
    std::vector<Triangle*> triangles;
    std::vector<Vector*> vertices;
    std::string line;

    while (std::getline(obj, line))
    {
        std::vector<std::string> tokens = split(line, ' ');
        if ((tokens.size() > 0) && (tokens[0] != "#"))
        {
            if (tokens[0] == "v") /* Add a vertex to the vertex list. */
            {
                Vector* v = new Vector(atof(tokens[1].c_str()),
                                       atof(tokens[2].c_str()),
                                       atof(tokens[3].c_str()));

                /* Apply rotation here. */
                /* IMPLEMENT ROTATION. */

                /* Scale the vector. */
                (*v) *= info.scaling;

                /* Translate it. */
                (*v) += info.translation;

                vertices.push_back(v);
            }
            else if (tokens[0] == "f") /* Parse this face (triangle). */
            {
                uint32_t v[3]; /* Vertices. */

                for (size_t t = 0; t < 3; ++t)
                {
                    std::vector<std::string> res = split(tokens[t + 1], '/');
                    v[t] = atoi(res[0].c_str()) - 1; /* Only need vertices. */
                }

                Triangle* t = new Triangle(*vertices[v[0]],
                                           *vertices[v[1]],
                                           *vertices[v[2]],
                                           info.modelID);

                triangles.push_back(t);
            }
        }
    }

    for (size_t t = 0; t < vertices.size(); ++t) delete vertices[t];

    return triangles;
}

Geometry::Geometry(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <Geometry>.\n");
    fprintf(stderr, "Loading '*/geometry.xml'.\n");

    std::fstream stream;
    pugi::xml_document doc;
    GetData("geometry.xml", stream);
    fprintf(stderr, "Parsing scene geometry...");

    ParseXML(doc, stream);
    fprintf(stderr, " done.\n");

    pugi::xml_node node = doc.child("geometry");
    leafSize = node.child("general").attribute("leaf").as_int();

    std::vector<Triangle*> triangleList;
    std::set<std::string> modelList;

    for (pugi::xml_node model : node.child("data").children("model"))
    {
        std::string modelPath = model.attribute("path").value();
        std::string modelID   = model.attribute("ID"  ).value();

        fprintf(stderr, "Parsing model '%s' [%s].\n", modelPath.c_str(),
                                                      modelID.c_str());

        std::fstream f;
        GetData("models/" + modelPath + ".obj", f);

        try
        {
            ModelInfo modelInfo;
            modelInfo.modelID = modelID;
            modelInfo.translation = parseVector(model.child("translation"));
            modelInfo.rotation    = parseVector(model.child("rotation"   ));
            modelInfo.scaling     = parseVector(model.child("scaling"    ));

            std::vector<Triangle*> data = ParseModel(f, modelInfo);
            triangleList.insert(triangleList.end(), data.begin(), data.end());

            modelList.insert(modelID);
        }
        catch (std::exception &e)
        {
            /* Something went wrong. */
            Error::Check(Error::IO, 0, true);
        }
    }

    count = triangleList.size();

    fprintf(stderr, "\nResolving model ID's.\n");

    /* Convert modelID to materialID. */
    for (size_t t = 0; t < count; ++t)
    {
        std::string m = triangleList[t]->model;
        triangleList[t]->material = std::distance(modelList.begin(),
                                                  modelList.find(m)) + 1;
    }

    fprintf(stderr, "\nTotal %u triangles.\n", count);
    fprintf(stderr, "Now building BVH.\n");

    uint32_t leafCount = 0, nodeCount = 0;
    BVHFlatNode* bvhTree = nullptr;

    BuildBVH(triangleList, leafSize, &leafCount, &nodeCount, &bvhTree);
    fprintf(stderr, "BVH successfully built, %u nodes.\n", nodeCount);
    fprintf(stderr, "Number of leaves: %u/%u.\n", leafCount, leafSize);
    fprintf(stderr, "\nNow compacting BVH.\n");

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

        this->nodes = CreateBuffer(params.context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_node) * nodeCount,
                                   rawNodes.get());

        fprintf(stderr, "BVH uploaded! Freeing resources.\n");
    }

    delete[] bvhTree;

    fprintf(stderr, "\nCompacting triangle list.\n");

    cl_triangle* raw = new cl_triangle[count];
    for (size_t t = 0; t < count; ++t) triangleList[t]->CL(raw + t);

    fprintf(stderr, "Triangles compacted, uploading to device...\n");

    this->triangles = CreateBuffer(params.context,
                                   CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_triangle) * count,
                                   raw);

    fprintf(stderr, "Triangle data uploaded! Freeing resources.\n");
    for (size_t t = 0; t < count; ++t) delete triangleList[t];
    delete [] raw;

    fprintf(stderr, "Initialization complete.\n\n");
    stream.close();
}

Geometry::~Geometry()
{
    // nothing here
}

void Geometry::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <triangles@Geometry> to index %u.\n", *index);
    BindArgument(params.kernel, triangles, (*index)++);
    fprintf(stderr, "Binding <nodes@Geometry> to index %u.\n", *index);
    BindArgument(params.kernel, nodes, (*index)++);
}

void Geometry::Update(size_t /* index */) { return; }

void* Geometry::Query(size_t query)
{
    return (query == Query::TriangleCount) ? &this->count : nullptr;
}
