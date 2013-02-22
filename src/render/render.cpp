#include <render/render.hpp>

struct __attribute__ ((packed)) cl_buffer
{
    cl_uint width, height;
};

bool DeviceParams::IsActive() { return false; }

void DeviceParams::Initialize()
{
    cl_buffer data = { params.width, params.height };

    cl_int error;
    this->buffer = cl::Buffer(params.context,
                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              sizeof(cl_buffer), &data, &error);
    Error::Check(Error::Memory, error);
}

void DeviceParams::Bind(cl_uint* slot)
{
	Error::Check(Error::Bind, params.kernel.setArg(*slot, this->buffer));
	(*slot)++;
}

void DeviceParams::Update(size_t index)
{
    return;
}

void* DeviceParams::Query(size_t query)
{
    return nullptr;
}

/******************************************************************************/

bool PixelBuffer::IsActive() { return false; }

void PixelBuffer::Initialize()
{
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

void PixelBuffer::Update(size_t index)
{
    this->index++;
}

void* PixelBuffer::Query(size_t query)
{
    return nullptr;
}

void PixelBuffer::Bind(cl_uint* slot)
{
	Error::Check(Error::Bind, params.kernel.setArg(*slot, this->buffer));
	(*slot)++;
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
        if (luminance > EPSILON) logAvg += log(luminance);
	}

	logAvg = exp(logAvg / (this->width * this->height));
    const float exposure = 0.18f;

	for (size_t t = 0; t < this->width * this->height; ++t)
	{
        this->pixels[t].Tonemap(logAvg, exposure);
	}
};

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

bool Tristimulus::IsActive() { return false; }

void Tristimulus::Initialize()
{
    cl_int error;
    cl::ImageFormat format(CL_RGBA, CL_FLOAT);
    this->buffer = cl::Image2D(params.context, CL_MEM_READ_ONLY, format,
                               SAMPLES, 1, 0, nullptr, &error);
    Error::Check(Error::Memory, error);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;


    cl::size_t<3> region;
    region[0] = SAMPLES;
    region[1] = 1;
    region[2] = 1;

    error = params.queue.enqueueWriteImage(this->buffer, CL_TRUE, origin,
                                           region, 0, 0, ColorMatchingCurve);
    Error::Check(Error::CLIO, error);
}

void Tristimulus::Bind(cl_uint* slot)
{
    Error::Check(Error::Bind, params.kernel.setArg(*slot, this->buffer));
	(*slot)++;
}

void Tristimulus::Update(size_t index)
{
    return;
}

void* Tristimulus::Query(size_t query)
{
    return nullptr;
}
