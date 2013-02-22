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

void DeviceParams::Bind(cl::Kernel kernel, cl_uint slot)
{
    Error::Check(Error::Bind, kernel.setArg(slot, this->buffer));
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

void PixelBuffer::Bind(cl::Kernel kernel, cl_uint slot)
{
    Error::Check(Error::Bind, kernel.setArg(slot, this->buffer));
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
    error = params.queue.enqueueWriteBuffer(this->buffer, CL_FALSE, 0,
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
