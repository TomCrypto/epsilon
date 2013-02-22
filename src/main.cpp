#include <iostream>
#include <cstdio>
#include <common/common.hpp>
#include <interface/interface.hpp>
#include <engine/epsilon.hpp>
#include <unistd.h>

using namespace std;

struct __attribute__ ((packed)) Scene
{
	uint res[4];
};

/* Gets the kernel from a text file. */
cl_program GetProgram(cl_context context, char* filePath)
{
    /* Try and open the file. */
    FILE* file = fopen(filePath, "rb");
    if (file == 0) return 0;

    /* Get the file size. */
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);      

    /* Read the file into a buffer. */
	cl_program program = 0;
    char* source = (char*)malloc(size + 1);
    if (fread(source, 1, size, file) > 0)
	{
    	source[size] = '\0';

    	/* Attempt to create the program. */
	    program = clCreateProgramWithSource(context, 1, (const char**)&source, 0, 0);
	}

   	/* Close the file and free. */
   	free(source);
   	fclose(file);

   	/* Return the program. */
   	return program;
}

int main(int argc, char* argv[])
{
	Interface* interface;

	try
	{
		interface = new Interface();
		interface->GetInput();

		Epsilon* engine = new Epsilon(interface->width, interface->height, interface->samples,
								 	  interface->platform, interface->device, interface->source, interface->output);

		//DeviceList list;
		//list.Initialize();

		//cl_device_id device = list.platforms[0].devices[0].ptr;
		interface->DisplayStatus("Rendering...", false);

		while (!engine->Finished())
		{
			engine->Execute();
			double* progress = (double*)engine->Query(Query::Progress);
			interface->progress = *progress;
			interface->DisplayProgress();
			double* etc = (double*)engine->Query(Query::EstimatedTime);
			interface->DisplayTime(*etc);

			double* elapsed = (double*)engine->Query(Query::ElapsedTime);
			uint32_t* triangles = (uint32_t*)engine->Query(Query::TriangleCount);
			interface->DisplayStatistics(*elapsed, *progress, *triangles);
		}

		interface->DisplayStatus("Rendering complete.", false);
		interface->Finish();
		delete engine;

#if 0

		/* Create context, queue, etc.. */
		cl_int error = 0;
		cl_context context = clCreateContext(0, 1, &device, 0, 0, &error);
		Error::Check(Error::Context, error);

		cl_command_queue queue = clCreateCommandQueue(context, device, 0, &error);
		Error::Check(Error::Queue, error);

		/* Get the local work group size for this device. */
		size_t localGroupSize;
		error = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(localGroupSize), &localGroupSize, 0);
		Error::Check(Error::DeviceInfo, error);

		cl_float3  test = {1.0, 2.0, 3.0};

		int width = interface->width;
		int height = interface->height;

		RenderBuffer render(context, width, height);

		Scene sceneData;
		sceneData.res[0] = width;
		sceneData.res[1] = height;

		float FOV = 45 * (3.14159265 / 180.0);
		Camera camera(Vector(0, 0, -15), Vector(0, -0, 1), FOV, context, queue);
		//cl_camera cam;
		//camera.CL(&cam);

		/* Get the maximum buffer size for this device. */
		cl_ulong bufferSize = sizeof(float) * 4 * width * height;

		/* And a small buffer, for the seed (beta * w bits long). */
		cl_mem seed = clCreateBuffer(context, CL_MEM_READ_ONLY, 8 * 4, 0, &error);
		Error::Check(Error::Memory, error);

		cl_mem scene = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(sceneData), 0, &error);
		Error::Check(Error::Memory, error);

		//cl_mem camBuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_camera), 0, 0);

		//clEnqueueWriteBuffer(queue, camBuf, CL_TRUE, 0, sizeof(cl_camera), &cam, 0, 0, 0);

		int triangleCount = 9; //13;
		cl_mem geometry = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(cl_triangle) * triangleCount, 0, 0);

		cl_triangle triangles[triangleCount];
		Triangle** good = new Triangle*[triangleCount];

		#if 0

		good[0] = new Triangle(Vector(-5, -5, -20),
		                       Vector(+5, -5, -20),
		                       Vector(+5, -5, +5),
		                       0);

		good[1] = new Triangle(Vector(-5, -5, -20),
		                       Vector(+5, -5, +5),
		                       Vector(-5, -5, +5),
		                       0);

		good[2] = new Triangle(Vector(-5, +5, -20),
		                       Vector(+5, +5, -20),
		                       Vector(+5, +5, +5),
		                       0);

		good[3] = new Triangle(Vector(-5, +5, -20),
		                       Vector(+5, +5, +5),
		                       Vector(-5, +5, +5),
		                       0);

		good[4] = new Triangle(Vector(-5, -5, -20),
		                       Vector(-5, -5, +5),
		                       Vector(-5, +5, +5),
		                       1);

		good[5] = new Triangle(Vector(-5, -5, -20),
		                       Vector(-5, +5, +5),
		                       Vector(-5, +5, -20),
		                       1);

		good[6] = new Triangle(Vector(+5, -5, -20),
		                       Vector(+5, -5, +5),
		                       Vector(+5, +5, +5),
		                       2);

		good[7] = new Triangle(Vector(+5, -5, -20),
		                       Vector(+5, +5, +5),
		                       Vector(+5, +5, -20),
		                       2);

		good[8] = new Triangle(Vector(-5, -5, +5),
		                       Vector(+5, -5, +5),
		                       Vector(+5, +5, +5),
		                       0);

		good[9] = new Triangle(Vector(-5, -5, +5),
		                       Vector(+5, +5, +5),
		                       Vector(-5, +5, +5),
		                       0);

		good[10] = new Triangle(Vector(-3, +4.99, -3),
		                        Vector(-3, +4.99, +3),
		                        Vector(+3, +4.99, +3),
		                        -1);

		good[11] = new Triangle(Vector(-3, +4.99, -3),
		                        Vector(+3, +4.99, +3),
		                        Vector(+3, +4.99, -3),
		                        -1);

		good[12] = new Triangle(Vector(-3, +4.99 + 20, -3),
		                        Vector(+3, +4.99 + 20, +3),
		                        Vector(+3, +4.99 + 20, -3),
		                        3);

		#endif

		good[0] = new Triangle(Vector(-5, -5, -20),
		                       Vector(+5, -5, -20),
		                       Vector(+5, -5, +5),
		                       0);

		good[1] = new Triangle(Vector(-5, -5, -20),
		                       Vector(+5, -5, +5),
		                       Vector(-5, -5, +5),
		                       0);

		good[2] = new Triangle(Vector(-5, -5, -20),
		                       Vector(-5, -5, +5),
		                       Vector(-5, +5, +5),
		                       1);

		good[3] = new Triangle(Vector(-5, -5, -20),
		                       Vector(-5, +5, +5),
		                       Vector(-5, +5, -20),
		                       1);

		good[4] = new Triangle(Vector(+5, -5, -20),
		                       Vector(+5, -5, +5),
		                       Vector(+5, +5, +5),
		                       2);

		good[5] = new Triangle(Vector(+5, -5, -20),
		                       Vector(+5, +5, +5),
		                       Vector(+5, +5, -20),
		                       2);

		good[6] = new Triangle(Vector(-5, -5, +5),
		                       Vector(+5, -5, +5),
		                       Vector(+5, +5, +5),
		                       0);

		good[7] = new Triangle(Vector(-5, -5, +5),
		                       Vector(+5, +5, +5),
		                       Vector(-5, +5, +5),
		                       0);

		good[8] = new Triangle(Vector(-3, +4.99 + 20, -3),
		                        Vector(+3, +4.99 + 20, +3),
		                        Vector(+3, +4.99 + 20, -3),
		                        3);

		#if 0

		good[1] = new Triangle(Vector(-1, 1, 0),
		                       Vector(-1, -1, 0),
		                       Vector(1, 1, 0), 0);

		good[2] = new Triangle(Vector(1, 1, 0),
		                       Vector(1, -1, 0),
		                       Vector(-1, -1, 0), 0); 

		good[0] = new Triangle(Vector(1, -1, 3),
		                       Vector(1, -3, 3),
		                       Vector(-1, -3, 3), 0);

		#endif

		// init here
		for (size_t t = 0; t < triangleCount; ++t)
		{
		    good[t]->CL(&triangles[t]);
		}
		
		clEnqueueWriteBuffer(queue, geometry, CL_TRUE, 0, sizeof(cl_triangle) * triangleCount, triangles, 0, 0, 0);
	
		clEnqueueWriteBuffer(queue, scene, CL_TRUE, 0, sizeof(sceneData), &sceneData, 0, 0, 0);

		/* Initialize the seed to some value. */
		void* blah = malloc(8 * 4);
		memset(blah, 0xba, 8 * 4);
		clEnqueueWriteBuffer(queue, seed, CL_TRUE, 0, 8 * 4, blah, 0, 0, 0);
		free(blah);

		/* Attempt to create this program. */
		cl_program program = GetProgram(context, "cl/epsilon.cl");

		/* Attempt to build the program. */
		error = clBuildProgram(program, 0, 0, "-I cl/", 0, 0);
		Error::Check(Error::Build, error);

		/* And create the kernel (finally) */
		cl_kernel kernel = clCreateKernel(program, "clmain", &error);
		Error::Check(Error::Kernel, error);
		if (kernel == 0)
		{
			throw std::runtime_error("Failed to create OpenCL kernel.");
		}

		render.Bind(kernel, 0);

		/*error = clSetKernelArg(kernel, 2, sizeof(seed), &seed);
		if (error != CL_SUCCESS)
		{
		    cout << "Failed to set kernel argument [" << error << "]." << endl;
		    return EXIT_FAILURE;
		}*/

		float wavelengths[3] = { 0.0f, 572.0f, 460.0f };
		Surface surfaces[3];

		for (size_t t = 0; t < 3; ++t)
		{
		    /* Get wavelength. */
		    for (size_t l = 0; l < 128; ++l)
		    {
		        if (wavelengths[t] == 0) surfaces[t].diffuse[l] = 1.0f;
		        else
		        {
		            float w = 380 + 400 * (float)l / 128;
		            float x = exp(-pow(w - wavelengths[t], 2.0f) * 0.001f);
		            surfaces[t].diffuse[l] = x;
		        }
		    }
		}
		


		Materials materials(context, queue, surfaces, 3);
		materials.Bind(kernel, 2);

		PRNG prng(context, queue);
		prng.Bind(kernel, 3);

		error = clSetKernelArg(kernel, 5, sizeof(scene), &scene);
		Error::Check(Error::Bind, error);

		error = clSetKernelArg(kernel, 1, sizeof(geometry), &geometry);
		//error = clSetKernelArg(kernel, 6, sizeof(camBuf), &camBuf);
		camera.Bind(kernel, 6);

		XYZCurve curve(context, queue);
		curve.Bind(kernel, 4);

		int sampleCount = interface->samples;

		/* Generate file. */
		size_t pixelCount = width * height;

		interface->DisplayStatus("Rendering...", false);
		for (size_t t = 0; t < sampleCount; ++t)
		{

			/* Generate one more batch of results. */
			error = clEnqueueNDRangeKernel(queue, kernel, 1, 0, &pixelCount, &localGroupSize, 0, 0, 0);
			Error::Check(Error::Execute, error);

		    prng.Renew(queue);
			clFinish(queue);

			interface->progress = (double)t / (sampleCount - 1);
			interface->DisplayProgress();
		}
		interface->DisplayStatus("Rendering complete.", false);

		render.Acquire(queue, CL_TRUE);
		render.ConvertToRGB();
		render.Tonemap();
		render.GammaCorrect();
		render.WriteToFile(interface->output);

		/* Clean up everything. */
		clReleaseKernel(kernel);
		clReleaseProgram(program);
		clReleaseMemObject(seed);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);

#endif
	}
	catch (exception& e)
	{
		interface->DisplayStatus(e.what(), true);
	}

	interface->Redraw();
	delete interface;

	return EXIT_SUCCESS;
}
