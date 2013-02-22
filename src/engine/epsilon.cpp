#include <engine/epsilon.hpp>

Epsilon::Epsilon(size_t width, size_t height, size_t samples,
                 cl::Platform platform, cl::Device device,
                 std::string source, std::string output)
{
    /* This will remain constant. */
    this->params.platform = platform;
    this->params.device   = device;
    this->params.source   = source;
    this->params.output   = output;
    this->params.width    = width;
    this->params.height   = height;
    this->params.samples  = samples;

    std::vector<cl::Device> devices;
    devices.push_back(device);

    cl_int error;
    this->params.context = cl::Context(devices,
                                       nullptr, nullptr, nullptr, &error);
    Error::Check(Error::Context, error);

    this->params.queue = cl::CommandQueue(this->params.context, device,
                                          0, &error);
    Error::Check(Error::Queue, error);

    std::string src = "#include <cl/epsilon.cl>";
    cl::Program::Sources sourceCode = cl::Program::Sources(1, std::make_pair(
                                      src.c_str(), src.length() + 1));

    this->params.program = cl::Program(this->params.context, sourceCode,
                                       &error);
    Error::Check(Error::Program, error);

    error = this->params.program.build(devices, "-I cl/");

    if (error != CL_SUCCESS)
    {
        std::string log;
        cl_int error_tmp;
        error_tmp = this->params.program.getBuildInfo(this->params.device,
                                                  CL_PROGRAM_BUILD_LOG, &log);
        Error::Check(Error::BuildLog, error_tmp);
        std::ofstream logfile("clc.log");
        logfile << log;
        logfile.close();
    }

    Error::Check(Error::Build, error);

    this->params.kernel = cl::Kernel(this->params.program, "clmain", &error);
    Error::Check(Error::Kernel, error);

    this->sampleIndex = 0;

    /* Add all kernel objects here... in the right order. */
    this->objects.push_back(new PixelBuffer (this->params));
	this->objects.push_back(new DeviceParams(this->params));
    this->objects.push_back(new Tristimulus (this->params));
    this->objects.push_back(new Geometry    (this->params));
    this->objects.push_back(new Camera      (this->params));
	this->objects.push_back(new PRNG        (this->params));
	this->objects.push_back(new Progress    (this->params));

	cl_uint slot = 0;
    for (int t = 0; t < this->objects.size(); ++t)
    {
        this->objects[t]->Initialize();
        this->objects[t]->Bind(&slot);
    }
}

Epsilon::~Epsilon()
{
    for (int t = 0; t < this->objects.size(); ++t)
    {
        delete this->objects[t];
    }
}

void Epsilon::Execute()
{
    for (int t = 0; t < this->objects.size(); ++t)
        if (this->objects[t]->IsActive())
            this->objects[t]->Update(this->sampleIndex);
    
	cl_int error;
	size_t local, global = this->params.width * this->params.height;
	error = this->params.device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE,
										&local);
	Error::Check(Error::DeviceQuery, error);

	cl::NDRange localSize(local), globalSize(global);
	error = this->params.queue.enqueueNDRangeKernel(this->params.kernel,
													cl::NullRange,
													globalSize,
													localSize);
	Error::Check(Error::Execute, error);
	this->params.queue.finish();

    this->sampleIndex++;
}

bool Epsilon::Finished() { return this->sampleIndex == this->params.samples; }

void* Epsilon::Query(size_t query)
{
    for (int t = 0; t < this->objects.size(); ++t)
    {
        void* ret = this->objects[t]->Query(query);
        if (ret != nullptr) return ret;
    }

    return nullptr;
}
