#pragma once

/* This file manages a render buffer, and provides useful functions to output *
 * this buffer to a .ppm file or to perform manipulations on the buffer, such *
 * as tone-mapping or gamma-correction. */

#include <common/common.hpp>
//#include <spectrum/spectrum.hpp>

/* This is pixel, which contains a three-color tristimulus value of any color *
 * space (XYZ or RGB) and a fourth format-dependent value, generally defining *  * the pixel's radiance. The formats are respectively XYZr and RGB. */
struct Pixel
{
    private:
        float x, y, z, r;
    public:
        /* Default constructor, sets the pixel to black by convention. */
        Pixel() { this->x = 0; this->y = 0; this->z = 0; this->r = 0; }

        /* Returns the pixel's XYZ or RGB value. */
        Vector XYZ() { return Vector(x, y, z); }
        Vector RGB() { return Vector(x, y, z); }

        /* Returns the pixel's luminance. Valid for RGB format only. */
        float Luminance()
        {
            return (this->x * 0.2645 +
                    this->y * 0.7243 + 
                    this->z * 0.0085);
        }

		/* Gamma-corrects the pixel. Valid for RGB format only. */
		void GammaCorrect()
		{
			if (this->x >= 0.018) this->x = (1.099 * pow(this->x, 0.45)) - 0.099;
        	else this->x *= ((1.099 * pow(0.018, 0.45)) - 0.099) / 0.018;
        	if (this->y >= 0.018) this->y = (1.099 * pow(this->y, 0.45)) - 0.099;
        	else this->y *= ((1.099 * pow(0.018, 0.45)) - 0.099) / 0.018;
        	if (this->z >= 0.018) this->z = (1.099 * pow(this->z, 0.45)) - 0.099;
        	else this->z *= ((1.099 * pow(0.018, 0.45)) - 0.099) / 0.018;
		}

        /* Converts the pixel from XYZr format to RGB format. */
        void XYZToRGB()
        {
            /* Normalize the XYZ color. */
            float sum = this->x + this->y + this->z;
            if (sum >= EPSILON)
            {
                this->x /= sum;
                this->y /= sum;
                this->z /= sum;
            }

            /* Uses the CIE XYZ -> RGB forward conversion matrix. */
            float R = +2.2878400 * x - 0.8333680 * y - 0.454471 * z;
            float G = -0.5116510 * x + 1.4227600 * y + 0.088893 * z;
            float B = +0.0057204 * x - 0.0159068 * y + 1.010190 * z;

            /* Constrain the new RGB color within the RGB gamut. */
            float w = std::min(0.0f, std::min(R, std::min(G, B)));
            R -= w; G -= w; B -= w;
            
            /* Multiply by radiance. */
            this->x = R * this->r; 
            this->y = G * this->r; 
            this->z = B * this->r;
            this->r = 0.0f;
        }

        /* This will tonemap the pixel based on a log-average luminance and an
         * exposure key. This is vaid for RGBr format only. */
        void Tonemap(float logAvg, float exposure)
        {
            float ky = exposure / logAvg;
            float mu = ky / (1.0f + this->Luminance() * ky);
            this->x *= mu;
            this->y *= mu;
            this->z *= mu;
        }

        /* This will convert the pixel's RGB values into byte range. */
        void ByteRGB(int *r, int *g, int *b)
        {
            *r = (int)(std::min(std::max(this->x, 0.0f), 1.0f) * 255);
            *g = (int)(std::min(std::max(this->y, 0.0f), 1.0f) * 255);
            *b = (int)(std::min(std::max(this->z, 0.0f), 1.0f) * 255);
        }
};

struct RenderBuffer
{
    private:
        size_t width, height, size;
        cl_mem buffer;
        Pixel *pixels;

    public:
        RenderBuffer(cl_context context, size_t width, size_t height);
        ~RenderBuffer();

        /* Bind buffer to kernel slot. */
        void Bind(cl_kernel kernel, cl_uint index);

        /* Read/write pixels from buffer. */
        void Acquire(cl_command_queue queue, cl_bool block);
        void Upload(cl_command_queue queue, cl_bool block);

        /* Convert current XYZn to unnormalized RGB colors. */
        void ConvertToRGB();

		/* Tone-maps current RGB colors to normalized RGB. */
		void Tonemap();

		/* Gamma-corrects the current RGB colors. */
		void GammaCorrect();

        /* Write the render AS-IS (no normalization) to a PPM file. */
        void WriteToFile(std::string path);
};
