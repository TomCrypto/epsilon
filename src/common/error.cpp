#include <common/error.hpp>

const std::string Error::PlatformQuery = "Failed to query OpenCL platforms.";
const std::string Error::DeviceQuery = "Failed to query OpenCL devices.";
const std::string Error::PlatformInfo = "Failed to query platform info.";
const std::string Error::DeviceInfo = "Failed to query device info.";
const std::string Error::Context = "Failed to initialize context.";
const std::string Error::Queue = "Failed to initialize command queue";
const std::string Error::Memory = "Failed to allocate memory object.";
const std::string Error::CLIO = "An OpenCL I/O failure occurred.";
const std::string Error::IO = "A generic I/O failure occurred.";
const std::string Error::Bind = "Failed to bind a memory object to kernel.";
const std::string Error::Program = "Failed to initialize program.";
const std::string Error::Build = "Failed to build program, see log file.";
const std::string Error::Kernel = "Failed to initialize kernel.";
const std::string Error::Execute = "Failed to execute kernel.";
const std::string Error::BuildLog = "Failed to retrieve CLC build log.";

void Error::Check(std::string msg, int code, bool override)
{
    if (!override) msg = "[" + std::to_string(code) + "] " + msg;
    if (override || (code != 0)) throw std::runtime_error(msg);
}

void QueryPlatforms(std::vector<cl::Platform>& platforms)
{
    Error::Check(Error::PlatformQuery, cl::Platform::get(&platforms));
}

void QueryDevices(cl::Platform& platform, std::vector<cl::Device>& devices)
{
    cl_int error = platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    Error::Check(Error::DeviceQuery, error);
}

void PlatformName(cl::Platform& platform, std::string& name)
{
    cl_int error = platform.getInfo(CL_PLATFORM_NAME, &name);
    Error::Check(Error::PlatformInfo, error);
}

void DeviceName(cl::Device& device, std::string& name)
{
    cl_int error = device.getInfo(CL_DEVICE_NAME, &name);
    Error::Check(Error::DeviceInfo, error);
}

cl::Context CreateContext(std::vector<cl::Device>& devices)
{
    cl_int error;
    cl::Context context = cl::Context(devices, nullptr, nullptr, nullptr,
                                      &error);
    Error::Check(Error::Context, error);
    return context;
}

cl::CommandQueue CreateQueue(cl::Context& context, cl::Device& device)
{
    cl_int error;
    cl::CommandQueue queue = cl::CommandQueue(context, device, 0, &error);
    Error::Check(Error::Queue, error);
    return queue;
}

cl::Program CreateProgram(cl::Context& context, cl::Program::Sources& code)
{
    cl_int error;
    cl::Program program = cl::Program(context, code, &error);
    Error::Check(Error::Program, error);
    return program;
}

cl::Kernel CreateKernel(cl::Program& program, const char* name)
{
    cl_int error;
    cl::Kernel kernel = cl::Kernel(program, name, &error);
    Error::Check(Error::Kernel, error);
    return kernel;
}

std::string GetBuildLog(cl::Program& program, cl::Device& device)
{
    std::string log;
    cl_int error = program.getBuildInfo(device, CL_PROGRAM_BUILD_LOG, &log);
    Error::Check(Error::BuildLog, error);
    return log;
}

size_t GetWorkGroupSize(cl::Kernel& kernel, cl::Device& device)
{
    size_t workGroupSize;
    cl_int error = kernel.getWorkGroupInfo(device, CL_KERNEL_WORK_GROUP_SIZE,
                                           &workGroupSize);
    
    Error::Check(Error::DeviceInfo, error);
    return workGroupSize;
}

void ExecuteKernel(cl::CommandQueue& queue, cl::Kernel& kernel,
                   cl::NDRange offset, cl::NDRange global, cl::NDRange local)
{
    cl_int error = queue.enqueueNDRangeKernel(kernel, offset, global, local);
    Error::Check(Error::Execute, error);
}

void FlushAndWait(cl::CommandQueue& queue)
{
    Error::Check(Error::Execute, queue.flush());
    Error::Check(Error::Execute, queue.finish());
}

cl::Buffer CreateBuffer(cl::Context& context, cl_mem_flags flags,
                        size_t size, void* hostptr)
{
    cl_int error;
    cl::Buffer buffer = cl::Buffer(context, flags, size, hostptr, &error);
    Error::Check(Error::Memory, error);
    return buffer;
}

void WriteToBuffer(cl::CommandQueue& queue, cl::Buffer& buffer, cl_bool block,
                   size_t offset, size_t size, const void* ptr)
{
    cl_int error = queue.enqueueWriteBuffer(buffer, block, offset, size, ptr);
    Error::Check(Error::CLIO, error);
}

void ReadFromBuffer(cl::CommandQueue& queue, const cl::Buffer& buffer,
                    cl_bool block, size_t offset, size_t size,
                    void* ptr)
{
    cl_int error = queue.enqueueReadBuffer(buffer, block, offset, size, ptr);
    Error::Check(Error::CLIO, error);
}

cl::Image2D CreateImage2D(cl::Context& context, cl_mem_flags flags,
                          cl::ImageFormat format, size_t width, size_t height,
                          void* hostptr)
{
    cl_int error;
    cl::Image2D image = cl::Image2D(context, flags, format, width, height, 0,
                              hostptr, &error);
    Error::Check(Error::Memory, error);
    return image;
}

void WriteToImage2D(cl::CommandQueue& queue, cl::Image2D& image, cl_bool block,
                    cl::size_t<3>& origin, cl::size_t<3>& region, void* ptr)
{
    cl_int error = queue.enqueueWriteImage(image, block, origin, region, 0, 0,
                                           ptr);
    Error::Check(Error::CLIO, error);
}

void ParseXML(pugi::xml_document& document, std::fstream &stream)
{
    if (!document.load(stream)) Error::Check(Error::IO, 0, true);
}
