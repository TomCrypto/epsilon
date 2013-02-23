#pragma once

#include <common/common.hpp>

/** @file architecture.hpp
  * @brief Engine architecture definitions.
  *
  * This file contains, in particular, the shared kernel object interface which
  * permits the engine to uniformly handle kernel arguments, requiring only
  * kernel-side code modifications.
**/

/** @struct EngineParams
  * @brief General engine parameters
  *
  * This contains runtime parameters such as platform/device information, and
  * user-provided options such as render width, height, passes, and so on.
**/
struct EngineParams
{
    /** @brief The OpenCL platform of the target device. **/
    cl::Platform platform;
    /** @brief The OpenCL device on which the engine will run. **/
    cl::Device device;
    /** @brief The OpenCL context used by the engine. **/
    cl::Context context;
    /** @brief The OpenCL command queue used by the engine. **/
    cl::CommandQueue queue;
    /** @brief The program used by the engine for rendering. **/
    cl::Program program;
    /** @brief The kernel used by the engine for rendering. **/
    cl::Kernel kernel;
    /** @brief The scene's source directory, absolute or relative. **/
    std::string source;
    /** @brief The engine's output file (a PPM image). **/
    std::string output;
    /** @brief The render width, in pixels. **/
    size_t width;
    /** @brief The render height, in pixels. **/
    size_t height;
    /** @brief The number of render passes (per pixel). **/
    size_t passes;
};

/** @class KernelObject
  * @brief Uniform kernel argument interface
  * 
  * Each kernel object manages its own memory (both host-side and device-side)
  * and binds itself to any number of kernel argument slots, provided by the
  * engine. Inputs to kernel objects come in two forms:
  * - Scene data, where the kernel object requests the contents of a particular
  *   file located inside the selected scene's directory (such as geometry data
  *   or material definitions). The object loads this data itself.
  * - Engine parameters, which are always passed to the kernel object even if
  *   not specifically required. This contains parameters selected by the user
  *   at runtime, such as render width and height, or render passes.
  *
  * All kernel objects will be notified and allowed to perform tasks whenever
  * the renderer is about to begin a new render pass, such as communicating
  * with the device, or even saving/reading from a file.
  *
  * All kernel objects can be queried for information by the engine. This
  * allows the renderer to, in turn, report information to the interface
  * for display. See the \c KernelObject::Query method for details.
**/
class KernelObject
{
    protected:
        /** @brief The engine parameters, provided by the engine. **/
        EngineParams& params;

        /** @brief Loads scene data based on an identifier.
          * @param id The identifier of the scene data to load.
          * @param data A streamable object containing the scene data.
          * @note If the scene data cannot be found, this will throw an
          *       exception, and the kernel object should probably die
          *       and cause the engine to fail if it cannot recover.
        **/
        void GetData(std::string id, std::ifstream& data)
        {
            try
            {
                data.open(params.source + "/" + id, std::ifstream::binary);
                if (!data.is_open()) Error::Check(Error::IO, 0, true);
            }
            catch (...)
            {
                Error::Check(Error::IO, 0, true);
            }
        }

    public:
        /** @brief Constructs the kernel object and passes engine parameters.
          * @param params The engine parameters.
          * @note All required resources are allocated here.
        **/
        KernelObject(EngineParams& params) : params(params) { }

        /** @brief Binds the kernel object to a kernel.
          * @param index A pointer to a running variable indicating the next
          *              available slot. The kernel object is to use up as
          *              many slots as it needs starting from this one, and
          *              increment the running variable for the next kernel
          *              object.
        **/
        virtual void Bind(cl_uint* index) = 0;

        /** @brief Updates the kernel object.
          * @param pass This indicates the pass the renderer is currently
          *             working on (this is zero-based, first pass is zero).
          * @note In total, this method will be called \c params.samples times.
        **/
        virtual void Update(size_t pass) = 0;

        /** @brief Queries the kernel object for information.
          * @param query An integer identifying the required information.
          * @returns A pointer to the requested information. The format of
          *          this information may vary and is assumed to be known
          *          to the caller. If the information is not found, returns
          *          \c nullptr (the null pointer).
          * @note Do not attempt to modify or deallocate the returned memory,
          *       as it belongs to the kernel object.
        **/
        virtual void* Query(size_t query) = 0;

        /** @brief Destroys the kernel object and frees all resources.
        **/
        virtual ~KernelObject() { }
};
