#include <camera/camera.hpp>

Camera::Camera(const Vector& pos, const Vector& dir, const float FOV,
               cl_context context, cl_command_queue queue)
{
    /* Create normal basis. */
    this->cameraPos = pos;

    Vector normal, tangent;
    Basis(normalize(dir), &normal, &tangent);

    /* Create four focal plane corners. */
    float z = 1.0f / tan(FOV * 0.5f);
    this->focalPlane[0] = Vector(-1.0f, +1.0f, z);
    this->focalPlane[1] = Vector(+1.0f, +1.0f, z);
    this->focalPlane[2] = Vector(+1.0f, -1.0f, z);
    this->focalPlane[3] = Vector(-1.0f, -1.0f, z);

    /* Transform via orthonormal basis. */
    for (size_t t = 0; t < 4; ++t)
    {
        this->focalPlane[t] = Transform(this->focalPlane[t], tangent,
                                                             normal,
                                                             dir);
        this->focalPlane[t] += this->cameraPos;
    }

    cl_int error;
    this->buffer = clCreateBuffer(context, CL_MEM_READ_ONLY,
                                  sizeof(cl_camera), 0, &error);
    if (!this->buffer)
    {
        throw std::runtime_error(Error(E_BUF, error));
    }

    cl_camera data;
    this->CL(&data);
    error = clEnqueueWriteBuffer(queue, this->buffer, CL_TRUE, 0,
                                        sizeof(cl_camera), &data, 0, 0, 0);

    if (error != CL_SUCCESS)
    {
        throw std::runtime_error(Error(E_WRITE, error));
    }
}

void Camera::Bind(cl_kernel kernel, cl_uint index)
{
    cl_int error = clSetKernelArg(kernel, index, sizeof(this->buffer),
                                  &this->buffer);
}

void Camera::CL(cl_camera *out)
{
    for (size_t t = 0; t < 4; ++t) this->focalPlane[t].CL(&out->p[t]);
    this->cameraPos.CL(&out->pos);
}

Camera::~Camera()
{
    clReleaseMemObject(this->buffer);
}
