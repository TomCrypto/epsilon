#include <engine/renderer.hpp>

Renderer::Renderer(size_t width, size_t height, size_t passes,
                   cl::Platform platform, cl::Device device,
                   std::string source, std::string output)
{
	params.platform = platform;
	params.device   = device;
	params.width    = width;
	params.height   = height;
	params.passes   = passes;
	params.source   = source;
	params.output   = output;
	currentPass = 0;

	/* For some reason, cl::Context requires a vector.. */
    std::vector<cl::Device> devices(&device, &device + 1);

    cl_int error;
    params.context = cl::Context(devices, nullptr, nullptr,
                                          nullptr, &error);
    Error::Check(Error::Context, error);

    params.queue = cl::CommandQueue(params.context, device,
                                    /* props. */ 0, &error);
    Error::Check(Error::Queue, error);

	/* This is a cheap trick to load programs. */
    const char* src = "#include <cl/epsilon.cl>";

	cl::Program::Sources data; /* We don't need multi-program support. */
	data = cl::Program::Sources(1, std::make_pair(src, strlen(src) + 1));

    params.program = cl::Program(params.context, data, &error);
    Error::Check(Error::Program, error);

    cl_int build_error = params.program.build(devices, "-I cl/");

    if (build_error != CL_SUCCESS)
    {
        std::string log;
        error = params.program.getBuildInfo(params.device,
                                            CL_PROGRAM_BUILD_LOG, &log);
        Error::Check(Error::BuildLog, error);
        std::ofstream logfile("clc.log");
        logfile << log;
        logfile.close();

        /* Only need this if the build fails. */
        Error::Check(Error::Build, build_error);
    }

    params.kernel = cl::Kernel(params.program, "clmain", &error);
    Error::Check(Error::Kernel, error);

    /* Add all kernel objects here, in order. */
    objects.push_back(new PixelBuffer (params));
	objects.push_back(new DeviceParams(params));
    objects.push_back(new Tristimulus (params));
    objects.push_back(new Geometry    (params));
    objects.push_back(new Camera      (params));
	objects.push_back(new PRNG        (params));
	objects.push_back(new Progress    (params));

	cl_uint slot = 0;
    for (size_t t = 0; t < objects.size(); ++t) objects[t]->Bind(&slot);
}

Renderer::~Renderer()
{
    for (size_t t = 0; t < objects.size(); ++t) delete objects[t];
}

bool Renderer::Execute()
{
    /* Guard to prevent doing redundant passes. */
	if (currentPass == params.passes) return true;

    for (size_t t = 0; t < objects.size(); ++t)
		objects[t]->Update(currentPass);
    
	cl_int error;
	size_t local, global = params.width * params.height;
	error = params.kernel.getWorkGroupInfo(params.device,
                                           CL_KERNEL_WORK_GROUP_SIZE,
                                           &local);
	Error::Check(Error::DeviceQuery, error);
	size_t offset = 0;

	while (global != 0)
	{
		size_t slice  = global - global % local;
		
		cl::NDRange localSize(local), globalSize(slice);
		cl::NDRange offsetRange(offset);

		error = params.queue.enqueueNDRangeKernel(params.kernel,
					    						  offsetRange,
												  globalSize,
												  localSize);
		Error::Check(Error::Execute, error);

		global = global % local;
		offset += slice;
		local /= 2;
	}

	this->params.queue.finish();

    return (++currentPass == params.passes);
}

void* Renderer::Query(size_t query)
{
    for (size_t t = 0; t < objects.size(); ++t)
    {
        void* ret = objects[t]->Query(query);
        if (ret != nullptr) return ret;
    }

	/* Not found */
    return nullptr;
}
