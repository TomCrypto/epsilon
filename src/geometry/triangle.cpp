#include <geometry/triangle.hpp>

/* This will create a triangle from three distinct vertices, and produce some *
 * precomputed values for use in other rendering subsystems.                  */
Triangle::Triangle(Vector p1, Vector p2, Vector p3, int material) : p1(p1), p2(p2), p3(p3)
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

