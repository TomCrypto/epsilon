#include <render/render.hpp>

RenderBuffer::RenderBuffer(cl_context context, size_t width, size_t height)
{
    this->width = width;
    this->height = height;
    this->size = this->width * this->height * sizeof(Pixel);
    
    /* Create pixel array and initialize to zero. */
    this->pixels = new Pixel[this->width * this->height];

    /* Create a device-side render buffer. */
    cl_int error;
    this->buffer = clCreateBuffer(context,
                                  CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                                  this->size, this->pixels, &error);
    
    if (this->buffer == nullptr) throw std::runtime_error(Error(E_BUF, error));
}

RenderBuffer::~RenderBuffer()
{
    clReleaseMemObject(this->buffer);
    delete[] this->pixels;
}

void RenderBuffer::Bind(cl_kernel kernel, cl_uint index)
{
    cl_int error = clSetKernelArg(kernel, index, sizeof(this->buffer),
                                  &this->buffer);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_BIND, error));
}

void RenderBuffer::Acquire(cl_command_queue queue, cl_bool block)
{
    cl_int error = clEnqueueReadBuffer(queue, this->buffer, block, 0,
                                       this->size, this->pixels, 0, 0, 0);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_READ, error));
}

void RenderBuffer::Upload(cl_command_queue queue, cl_bool block)
{
    cl_int error = clEnqueueWriteBuffer(queue, this->buffer, block, 0,
                                        this->size, this->pixels, 0, 0, 0);

    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_WRITE, error));
}

void RenderBuffer::ConvertToRGB()
{
    /* Go over each pixel. */
    for (size_t t = 0; t < this->width * this->height; ++t)
    {
        this->pixels[t].XYZToRGB();
    }
}

void RenderBuffer::Tonemap()
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

void RenderBuffer::GammaCorrect()
{
	for (size_t t = 0; t < this->width * this->height; ++t)
	{
		this->pixels[t].GammaCorrect();
	} 
}

void RenderBuffer::WriteToFile(std::string path)
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