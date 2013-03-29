#include <render/render.hpp>
#include <common/version.hpp>

#include <cmath>

struct cl_buffer
{
    cl_uint width, height;
};

PixelBuffer::PixelBuffer(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <PixelBuffer>...\n");

    cl_buffer data = { (cl_uint)params.width, (cl_uint)params.height };
    
    rp = CreateBuffer(params.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                      sizeof(cl_buffer), &data);

    size_t floatCount = params.width * params.height * 4;

    pixels = new float[floatCount];

    for (size_t t = 0; t < floatCount; ++t) pixels[t] = 0.0f;

    pb = CreateBuffer(params.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      floatCount * sizeof(float), pixels);

    fprintf(stderr, "Initialization complete.\n\n");
}

PixelBuffer::~PixelBuffer()
{
    Acquire(params);
    WriteToFile(params.output);

    delete[] pixels;
}

void PixelBuffer::Update(size_t /* index */)
{
    return;
}

void* PixelBuffer::Query(size_t /* query */)
{
    return nullptr;
}

void PixelBuffer::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <pb@PixelBuffer> to index %u.\n", *index);
    BindArgument(params.kernel, pb, (*index)++);
    fprintf(stderr, "Binding >rp@PixelBuffer> to index %u.\n", *index);
    BindArgument(params.kernel, rp, (*index)++);
}

void PixelBuffer::Acquire(EngineParams& params)
{
    size_t bufSize = params.width * params.height * sizeof(float) * 4;
    ReadFromBuffer(params.queue, pb, CL_TRUE, 0, bufSize, pixels);
}

void PixelBuffer::WriteToFile(std::string path)
{
    std::ofstream file;
    file.open(path, std::ios::out | std::ios::binary);

    /* Print Radiance's HDR header. */
    file << "#?RADIANCE" << std::endl;
    file << "SOFTWARE=epsilon " << GetRendererVersion() << std::endl;
    file << "FORMAT=32-bit_rle_xyze" << std::endl << std::endl;

    file << "-Y " << params.height << " +X " << params.width << std::endl;

    for (size_t t = 0; t < params.width * params.height; ++t)
    {
        /* Read pixel components. */
        float a = pixels[t * 4 + 0];
        float b = pixels[t * 4 + 1];
        float c = pixels[t * 4 + 2];
        float n = pixels[t * 4 + 3];

        /* Scaling. */
        if (n != 0.0f)
        {
            a /= n;
            b /= n;
            c /= n;
        }

        /* Convert to shared-exponent (see Radiance: HDR format) encoding. */
        uint8_t e = ceil(log(std::max(a, std::max(b, c))) / log(2.0f) + 128);
        uint8_t x = floor((256 * a) / pow(2.0f, e - 128));
        uint8_t y = floor((256 * b) / pow(2.0f, e - 128));
        uint8_t z = floor((256 * c) / pow(2.0f, e - 128));

        /* Write the bytes out to the image. */
        file.write((char*)&x, sizeof(uint8_t));
        file.write((char*)&y, sizeof(uint8_t));
        file.write((char*)&z, sizeof(uint8_t));
        file.write((char*)&e, sizeof(uint8_t));
    }

    file.close();
}

/******************************************************************************/

/* A XYZp color format (padding float at the end). */
struct XYZp
{
    float x, y, z, p;
    XYZp(float x, float y, float z) : x(x), y(y), z(z), p(1.0f) { };
};

Tristimulus::Tristimulus(EngineParams& params) : KernelObject(params)
{
    size_t res = Spectral::Resolution();
    fprintf(stderr, "Initializing <Tristimulus>.\n");
    fprintf(stderr, "Spectral resolution: %.1fnm.\n", 400.0 / (res - 1));

    cl::ImageFormat format(CL_RGBA, CL_FLOAT);
    this->buffer = CreateImage2D(params.context, CL_MEM_READ_ONLY, format,
                                 res, 1);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = res;
    region[1] = 1;
    region[2] = 1;

    WriteToImage2D(params.queue, this->buffer, CL_TRUE, origin, region,
                   Spectral::Curve());
    fprintf(stderr, "Initialization complete.\n\n");
}

void Tristimulus::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@Tristimulus> to index %u.\n", *index);
    BindArgument(params.kernel, buffer, (*index)++);
}

void Tristimulus::Update(size_t /* index */)
{
    return;
}

void* Tristimulus::Query(size_t /* query */)
{
    return nullptr;
}
