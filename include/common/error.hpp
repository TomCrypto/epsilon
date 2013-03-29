#pragma once

#include <exception>
#include <stdexcept>
#include <string>

/** @file error.hpp
  * @brief Error code declarations.
**/

/** @namespace Error
  *
  * @brief Error handling utilities.
**/
namespace Error
{
    /** @brief The available OpenCL platforms could not be enumerated. **/
    extern const std::string PlatformQuery;
    /** @brief The available OpenCL devices could not be enumerated. **/
    extern const std::string DeviceQuery;
    /** @brief Platform information could not be retrieved. **/
    extern const std::string PlatformInfo;
    /** @brief Device information could not be retrieved. **/
    extern const std::string DeviceInfo;
    /** @brief An OpenCL context could not be initialized. **/
    extern const std::string Context;
    /** @brief An OpenCL command queue could not be initialized. **/
    extern const std::string Queue;
    /** @brief Failed to allocate an OpenCL memory object (buffer, image). **/
    extern const std::string Memory;
    /** @brief Failed to successfully complete an OpenCL I/O operation. **/
    extern const std::string CLIO;
    /** @brief Failed to successfully complete a generic I/O operation. **/
    extern const std::string IO;
    /** @brief Failed to bind an OpenCL memory object to an OpenCL kernel. **/
    extern const std::string Bind;
    /** @brief Failed to initialize an OpenCL program object. **/
    extern const std::string Program;
    /** @brief Failed to build an OpenCL program (CL compiler failure). **/
    extern const std::string Build;
    /** @brief Failed to initialize an OpenCL kernel from a built program. **/
    extern const std::string Kernel;
    /** @brief Kernel execution on the selected device did not succeed. **/
    extern const std::string Execute;
    /** @brief The CLC build log could not be retrieved. **/
    extern const std::string BuildLog;

    /** @brief Throws a formatted exception based on an error code.
      * @param msg The message to use, such as Error::Memory.
      * @param code The error code presumably returned by the operation.
      * @param override Whether to use the given code, if this is \c true then
      *                 an exception will always be thrown and \c code will be
      *                 ignored as well as omitted from the exception message.
      * @note If \c override is \c false, an exception will be thrown iff \c
      *       code \c != \c 0 (this is also consistent with \c CL_SUCCESS).
    **/
    void Check(std::string msg, int code, bool override = false);
}

/* The following are exception-enabled wrappers around various functions. */
#include <misc/pugixml.hpp>
#include <CL/cl.hpp>
#include <fstream>

void QueryPlatforms(std::vector<cl::Platform>& platforms);
void QueryDevices(cl::Platform& platform, std::vector<cl::Device>& devices);
void PlatformName(cl::Platform& platform, std::string& name);
void DeviceName(cl::Device& device, std::string& name);
cl::Context CreateContext(std::vector<cl::Device>& devices);
cl::CommandQueue CreateQueue(cl::Context& context, cl::Device& device);
cl::Program CreateProgram(cl::Context& context, cl::Program::Sources& code);
cl::Kernel CreateKernel(cl::Program& program, const char* name);
std::string GetBuildLog(cl::Program& program, cl::Device& device);
size_t GetWorkGroupSize(cl::Kernel& kernel, cl::Device& device);
void ExecuteKernel(cl::CommandQueue& queue, cl::Kernel& kernel,
                   cl::NDRange offset, cl::NDRange global, cl::NDRange local);
void FlushAndWait(cl::CommandQueue& queue);

template <typename T>
void BindArgument(cl::Kernel& kernel, T& buffer, cl_uint index)
{
    Error::Check(Error::Bind, kernel.setArg(index, buffer));   
}

cl::Buffer CreateBuffer(cl::Context& context, cl_mem_flags flags,
                        size_t size, void* hostptr = nullptr);
void WriteToBuffer(cl::CommandQueue& queue, cl::Buffer& buffer, cl_bool block,
                   size_t offset, size_t size, const void* ptr);
void ReadFromBuffer(cl::CommandQueue& queue, const cl::Buffer& buffer,
                    cl_bool block, size_t offset, size_t size,
                    void* ptr);
cl::Image2D CreateImage2D(cl::Context& context, cl_mem_flags flags,
                          cl::ImageFormat format, size_t width, size_t height,
                          void* hostptr = nullptr);
void WriteToImage2D(cl::CommandQueue& queue, cl::Image2D& image, cl_bool block,
                    cl::size_t<3>& origin, cl::size_t<3>& region, void* ptr);

void ParseXML(pugi::xml_document& document, std::fstream& stream);
