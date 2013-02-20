#include <error/error.hpp>

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
const std::string Error::Build = "Failed to build program.";
const std::string Error::Kernel = "Failed to initialize kernel.";
const std::string Error::Execute = "Failed to execute kernel.";

void Error::Check(std::string msg, int code, bool override)
{
    if (!override) msg = "[" + std::to_string(code) + "] " + msg;
    if (override || (code != 0)) throw std::runtime_error(msg);
}
