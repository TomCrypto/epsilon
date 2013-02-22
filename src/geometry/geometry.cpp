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

struct cl_info
{
	cl_uint triangleCount;
};

bool Geometry::IsActive() { return false; }

void Geometry::Initialize()
{
    /* READ GEOMETRY DATA HERE. */

	this->list.push_back(Triangle(Vector(-5, -5, -20), Vector(+5, -5, -20), Vector(+5, -5, +5), 0));
	this->list.push_back(Triangle(Vector(-5, -5, -20), Vector(+5, -5, +5), Vector(-5, -5, +5), 0));
	this->list.push_back(Triangle(Vector(-5, -5, -20), Vector(-5, -5, +5), Vector(-5, +5, +5), 440));
	this->list.push_back(Triangle(Vector(-5, -5, -20), Vector(-5, +5, +5), Vector(-5, +5, -20), 440));
	this->list.push_back(Triangle(Vector(+5, -5, -20), Vector(+5, -5, +5), Vector(+5, +5, +5), 530));
	this->list.push_back(Triangle(Vector(+5, -5, -20), Vector(+5, +5, +5), Vector(+5, +5, -20), 530));
	this->list.push_back(Triangle(Vector(-5, -5, +5), Vector(+5, -5, +5), Vector(+5, +5, +5), 0));
	this->list.push_back(Triangle(Vector(-5, -5, +5), Vector(+5, +5, +5), Vector(-5, +5, +5), 0));

    /* END READ DATA. */

    this->count = this->list.size();
    cl_triangle raw[this->count];
    for (size_t t = 0; t < this->count; ++t) list[t].CL(raw + t);

    cl_int error;
    this->triangles = cl::Buffer(params.context,
                                 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                 sizeof(cl_triangle) * this->count,
                                 raw, &error);
    Error::Check(Error::Memory, error);

	cl_info info;
	info.triangleCount = this->count;

	this->sceneInfo = cl::Buffer(params.context,
						     	 CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
								 sizeof(cl_info), &info, &error);
	Error::Check(Error::Memory, error);
}

void Geometry::Bind(cl_uint* slot)
{
    Error::Check(Error::Bind, params.kernel.setArg(*slot, this->triangles));
    (*slot)++;
    Error::Check(Error::Bind, params.kernel.setArg(*slot, this->sceneInfo));
    (*slot)++;
}

void Geometry::Update(size_t index)
{
    return;
}

void* Geometry::Query(size_t query)
{
    if (query == Query::TriangleCount) return &this->count;
    return nullptr;
}
