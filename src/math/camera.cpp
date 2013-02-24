#include <math/camera.hpp>

struct cl_data
{
    cl_float4 p[4];
    cl_float4 pos;
};

Camera::Camera(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <Camera>...");

    /* READ DATA HERE. */
    Vector cameraPos = Vector(0, 0, -14.9);
    Vector dir = Vector(0, 0, 1);
    float FOV = 45 * (3.14169265 / 180);
    /* END READ DATA. */

    Vector normal, tangent;
    Basis(normalize(dir), &normal, &tangent);

    Vector focalPlane[4];
    float z = 1.0f / tan(FOV * 0.5f);
    focalPlane[0] = Vector(-1.0f, +1.0f, z);
    focalPlane[1] = Vector(+1.0f, +1.0f, z);
    focalPlane[2] = Vector(+1.0f, -1.0f, z);
    focalPlane[3] = Vector(-1.0f, -1.0f, z);

    for (size_t t = 0; t < 4; ++t)
    {
        focalPlane[t] = Transform(focalPlane[t], tangent, normal, dir);
        focalPlane[t] += cameraPos;
    }

    cl_int error;
    this->buffer = cl::Buffer(params.context, CL_MEM_READ_ONLY,
                              sizeof(cl_data), nullptr, &error);
    Error::Check(Error::Memory, error);

    cl_data data;
    for (size_t t = 0; t < 4; ++t) focalPlane[t].CL(&data.p[t]);
    cameraPos.CL(&data.pos);

    error = params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                            sizeof(cl_data), &data);
    Error::Check(Error::CLIO, error);

    fprintf(stderr, " complete!\n\n");
}

void Camera::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@Camera> to slot %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->buffer));
	(*index)++;
}

void Camera::Update(size_t /* index */)
{
    return;
}

void* Camera::Query(size_t /* query */)
{
    return nullptr;
}
