#include <render/render.hpp>

struct __attribute__ ((packed)) cl_buffer
{
    cl_uint width, height, pass;
};

DeviceParams::DeviceParams(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <DeviceParams>...");
    cl_buffer data = { (cl_uint)params.width, (cl_uint)params.height, 0 };

    cl_int error;
    this->buffer = cl::Buffer(params.context,
                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              sizeof(cl_buffer), &data, &error);
    Error::Check(Error::Memory, error);
    fprintf(stderr, " complete.\n\n");
}

void DeviceParams::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@DeviceParams> to slot %u.\n", *index);
	Error::Check(Error::Bind, params.kernel.setArg(*index, this->buffer));
	(*index)++;
}

void DeviceParams::Update(size_t index)
{
	cl_uint pass = (cl_uint)index;
	cl_buffer x = { (cl_uint)params.width, (cl_uint)params.height, pass };
	params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                    sizeof(cl_buffer), &x);

    return;
}

void* DeviceParams::Query(size_t /* query */)
{
    return nullptr;
}

/******************************************************************************/

PixelBuffer::PixelBuffer(EngineParams& params) : KernelObject(params)
{
    fprintf(stderr, "Initializing <PixelBuffer>...");
    this->width = params.width;
    this->height = params.height;
    this->size = this->width * this->height * sizeof(Pixel);

    this->pixels = new Pixel[this->width * this->height];

    cl_int error;
    this->buffer = cl::Buffer(params.context,
                              CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                              this->size, this->pixels, &error);
    Error::Check(Error::Memory, error);

    this->index = 0;
    fprintf(stderr, " complete.\n\n");
}

PixelBuffer::~PixelBuffer()
{
    Acquire(params);
    ConvertToRGB();
    Tonemap();
    GammaCorrect();
    WriteToFile(params.output);

    delete[] this->pixels;
}

void PixelBuffer::Update(size_t /* index */)
{
    this->index++;
}

void* PixelBuffer::Query(size_t /* query */)
{
    return nullptr;
}

void PixelBuffer::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@PixelBuffer> to slot %u.\n", *index);
	Error::Check(Error::Bind, params.kernel.setArg(*index, this->buffer));
	(*index)++;
}

void PixelBuffer::Acquire(const EngineParams& params)
{
    cl_int error;
    error = params.queue.enqueueReadBuffer(this->buffer, CL_TRUE, 0,
                                           this->size, this->pixels);
	Error::Check(Error::CLIO, error);
}

void PixelBuffer::Upload(const EngineParams& params)
{
    cl_int error;
    error = params.queue.enqueueWriteBuffer(this->buffer, CL_TRUE, 0,
                                            this->size, this->pixels);
	Error::Check(Error::CLIO, error);
}

void PixelBuffer::ConvertToRGB()
{
    /* Go over each pixel. */
    for (size_t t = 0; t < this->width * this->height; ++t)
    {
        this->pixels[t].XYZToRGB();
    }
}

void PixelBuffer::Tonemap()
{
	float logAvg = 0.0f;
	for (size_t t = 0; t < this->width * this->height; ++t)
    {
        float luminance = this->pixels[t].Luminance();
        logAvg += log(luminance + EPSILON);
	}

	logAvg = exp(logAvg / (this->width * this->height));
    const float exposure = 0.18f;

	for (size_t t = 0; t < this->width * this->height; ++t)
	{
        this->pixels[t].Tonemap(logAvg, exposure);
	}
}

void PixelBuffer::GammaCorrect()
{
	for (size_t t = 0; t < this->width * this->height; ++t)
	{
		this->pixels[t].GammaCorrect();
	} 
}

void PixelBuffer::WriteToFile(std::string path)
{
    std::ofstream file;
    file.open(path);

    /* Write PPM header. */
    file << "P3" << std::endl << this->width << " " << this->height << " 255";
    file << std::endl;

    /* Write pixels as they are in the buffer (may not be in RGB format). */
    for (size_t t = 0; t < this->width * this->height; ++t)
    {
        int r, g, b;
        this->pixels[t].ByteRGB(&r, &g, &b);

        file << r << " " << g << " " << b << " ";
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

    cl_int error;
    cl::ImageFormat format(CL_RGBA, CL_FLOAT);
    this->buffer = cl::Image2D(params.context, CL_MEM_READ_ONLY, format,
                               res, 1, 0, nullptr, &error);
    Error::Check(Error::Memory, error);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = res;
    region[1] = 1;
    region[2] = 1;

    error = params.queue.enqueueWriteImage(this->buffer, CL_TRUE, origin,
                                           region, 0, 0, Spectral::Curve());
    Error::Check(Error::CLIO, error);
    fprintf(stderr, "Initialization complete.\n\n");
}

void Tristimulus::Bind(cl_uint* index)
{
    fprintf(stderr, "Binding <buffer@Tristimulus> to slot %u.\n", *index);
    Error::Check(Error::Bind, params.kernel.setArg(*index, this->buffer));
	(*index)++;
}

void Tristimulus::Update(size_t /* index */)
{
    return;
}

void* Tristimulus::Query(size_t /* query */)
{
    return nullptr;
}
