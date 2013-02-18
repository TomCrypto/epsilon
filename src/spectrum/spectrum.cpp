#include <spectrum/spectrum.hpp>

/* A XYZp color format (padding float at the end). */
struct XYZp
{
	float x, y, z, p;
	XYZp(float x, float y, float z) : x(x), y(y), z(z), p(1.0f) { };
};

/* Number of wavelength samples in the color-matching curve. */
#define SAMPLES 81

/* The CIE color matching curve, at 5nm intervals. */
static XYZp ColorMatchingCurve[SAMPLES] = {
    XYZp(0.0014,0.0000,0.0065), XYZp(0.0022,0.0001,0.0105), XYZp(0.0042,0.0001,0.0201),
    XYZp(0.0076,0.0002,0.0362), XYZp(0.0143,0.0004,0.0679), XYZp(0.0232,0.0006,0.1102),
    XYZp(0.0435,0.0012,0.2074), XYZp(0.0776,0.0022,0.3713), XYZp(0.1344,0.0040,0.6456),
    XYZp(0.2148,0.0073,1.0391), XYZp(0.2839,0.0116,1.3856), XYZp(0.3285,0.0168,1.6230),
    XYZp(0.3483,0.0230,1.7471), XYZp(0.3481,0.0298,1.7826), XYZp(0.3362,0.0380,1.7721),
    XYZp(0.3187,0.0480,1.7441), XYZp(0.2908,0.0600,1.6692), XYZp(0.2511,0.0739,1.5281),
    XYZp(0.1954,0.0910,1.2876), XYZp(0.1421,0.1126,1.0419), XYZp(0.0956,0.1390,0.8130),
    XYZp(0.0580,0.1693,0.6162), XYZp(0.0320,0.2080,0.4652), XYZp(0.0147,0.2586,0.3533),
    XYZp(0.0049,0.3230,0.2720), XYZp(0.0024,0.4073,0.2123), XYZp(0.0093,0.5030,0.1582),
    XYZp(0.0291,0.6082,0.1117), XYZp(0.0633,0.7100,0.0782), XYZp(0.1096,0.7932,0.0573),
    XYZp(0.1655,0.8620,0.0422), XYZp(0.2257,0.9149,0.0298), XYZp(0.2904,0.9540,0.0203),
    XYZp(0.3597,0.9803,0.0134), XYZp(0.4334,0.9950,0.0087), XYZp(0.5121,1.0000,0.0057),
    XYZp(0.5945,0.9950,0.0039), XYZp(0.6784,0.9786,0.0027), XYZp(0.7621,0.9520,0.0021),
    XYZp(0.8425,0.9154,0.0018), XYZp(0.9163,0.8700,0.0017), XYZp(0.9786,0.8163,0.0014),
    XYZp(1.0263,0.7570,0.0011), XYZp(1.0567,0.6949,0.0010), XYZp(1.0622,0.6310,0.0008),
    XYZp(1.0456,0.5668,0.0006), XYZp(1.0026,0.5030,0.0003), XYZp(0.9384,0.4412,0.0002),
    XYZp(0.8544,0.3810,0.0002), XYZp(0.7514,0.3210,0.0001), XYZp(0.6424,0.2650,0.0000),
    XYZp(0.5419,0.2170,0.0000), XYZp(0.4479,0.1750,0.0000), XYZp(0.3608,0.1382,0.0000),
    XYZp(0.2835,0.1070,0.0000), XYZp(0.2187,0.0816,0.0000), XYZp(0.1649,0.0610,0.0000),
    XYZp(0.1212,0.0446,0.0000), XYZp(0.0874,0.0320,0.0000), XYZp(0.0636,0.0232,0.0000),
    XYZp(0.0468,0.0170,0.0000), XYZp(0.0329,0.0119,0.0000), XYZp(0.0227,0.0082,0.0000),
    XYZp(0.0158,0.0057,0.0000), XYZp(0.0114,0.0041,0.0000), XYZp(0.0081,0.0029,0.0000),
    XYZp(0.0058,0.0021,0.0000), XYZp(0.0041,0.0015,0.0000), XYZp(0.0029,0.0010,0.0000),
    XYZp(0.0020,0.0007,0.0000), XYZp(0.0014,0.0005,0.0000), XYZp(0.0010,0.0004,0.0000),
    XYZp(0.0007,0.0002,0.0000), XYZp(0.0005,0.0002,0.0000), XYZp(0.0003,0.0001,0.0000),
    XYZp(0.0002,0.0001,0.0000), XYZp(0.0002,0.0001,0.0000), XYZp(0.0001,0.0000,0.0000),
    XYZp(0.0001,0.0000,0.0000), XYZp(0.0001,0.0000,0.0000), XYZp(0.0000,0.0000,0.0000)};

XYZCurve::XYZCurve(cl_context context, cl_command_queue queue)
{
    cl_image_format format = { CL_RGBA, CL_FLOAT };
    cl_image_desc desc;
    desc.image_type = CL_MEM_OBJECT_IMAGE1D;
    desc.image_width = SAMPLES; 
    desc.image_row_pitch = 0;
    desc.image_slice_pitch = 0;
    desc.num_mip_levels = 0;
    desc.num_samples = 0;
    desc.buffer = 0;

    cl_int error;
    this->curve = clCreateImage(context, CL_MEM_READ_ONLY, &format, &desc,
                                nullptr, &error);

    if (this->curve == nullptr) throw std::runtime_error(Error(E_BUF, error));

    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { SAMPLES, 1, 1 };
    error = clEnqueueWriteImage(queue, this->curve, CL_TRUE, origin, region,
                                0, 0, ColorMatchingCurve, 0, 0, 0);

   if (error != CL_SUCCESS) throw std::runtime_error(Error(E_WRITE, error));
}

void XYZCurve::Bind(cl_kernel kernel, cl_uint index)
{
    cl_int error = clSetKernelArg(kernel, index, sizeof(this->curve),
                                  &this->curve);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_BIND, error));
}

XYZCurve::~XYZCurve()
{
    clReleaseMemObject(this->curve);
}
