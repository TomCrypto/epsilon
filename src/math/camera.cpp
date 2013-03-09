#include <math/camera.hpp>
#include <math/vector.hpp>
#include <misc/xmlutils.hpp>
#include <misc/pugixml.hpp>

struct cl_data
{
    cl_float4 p[4];     /* Focal plane.                                   */
    cl_float4 pos;      /* Camera position.                               */
    cl_float4 up, left; /* "Up" and "left" vectors (of the camera basis). */
    cl_float spread;    /* The focal spread, or aperture radius.          */
};

Camera::Camera(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <Camera>.\n");
    fprintf(stderr, "Loading '*/camera.xml'.\n");

    std::fstream stream;
    pugi::xml_document doc;
    GetData("camera.xml", stream);
    fprintf(stderr, "Parsing camera parameters...");

    if (!doc.load(stream)) Error::Check(Error::IO, 0, true);
    fprintf(stderr, " done.\n");

    pugi::xml_node node = doc.child("camera");
    Vector cameraPos = parseVector(node.child("position"));
    Vector cameraTarget = parseVector(node.child("target"));
    Vector dir = normalize(cameraTarget - cameraPos);

    node = node.child("misc");
    float FOV = (PI / 180.0f) * node.attribute("fov").as_float();

    node = node.parent();
    node = node.child("focal"); /* Depth-of-field params. */
    float focalSpread = node.attribute("spread").as_float();
    float focalLength = node.attribute("length").as_float();

    fprintf(stderr, "Camera position = (%.2f, %.2f, %.2f).\n", cameraPos.x,
                                                               cameraPos.y,
                                                               cameraPos.z);

    fprintf(stderr, "Camera target = (%.2f, %.2f, %.2f).\n", cameraTarget.x,
                                                             cameraTarget.y,
                                                             cameraTarget.z);

    fprintf(stderr, "Field of view = %.2f radians.\n", FOV);
    fprintf(stderr, "Focal length = %.2f.\n", focalLength);
    fprintf(stderr, "Focal spread = %.2f.\n", focalSpread);

    Vector normal, tangent;
    Basis(dir, &normal, &tangent);

    Vector focalPlane[4];
    float z = 1.0f / tan(FOV * 0.5f);
    focalPlane[0] = normalize(Vector(-1.0f, +1.0f, z));
    focalPlane[1] = normalize(Vector(+1.0f, +1.0f, z));
    focalPlane[2] = normalize(Vector(+1.0f, -1.0f, z));
    focalPlane[3] = normalize(Vector(-1.0f, -1.0f, z));

    for (size_t t = 0; t < 4; ++t)
    {
        /* Transform the focal plane to camera space (= "rotate" it) */
        focalPlane[t] = Transform(focalPlane[t], tangent, normal, dir);
        focalPlane[t] = focalPlane[t] * focalLength + cameraPos;
    }

    cl_int error;
    this->buffer = cl::Buffer(params.context, CL_MEM_READ_ONLY,
                              sizeof(cl_data), nullptr, &error);
    Error::Check(Error::Memory, error);

    cl_data data;
    for (size_t t = 0; t < 4; ++t) focalPlane[t].CL(&data.p[t]);
    cameraPos.CL(&data.pos);
    normal.CL(&data.up);
    tangent.CL(&data.left);
    data.spread = focalSpread;

    error = params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                            sizeof(cl_data), &data);
    Error::Check(Error::CLIO, error);

    fprintf(stderr, "Initialization complete.\n\n");
    stream.close();
}

void Camera::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@Camera> to slot %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->buffer));
    (*index)++;
}

void Camera::Update(size_t /* index */) { return; }
void* Camera::Query(size_t /* query */) { return nullptr; }
