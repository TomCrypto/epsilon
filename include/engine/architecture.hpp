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
  * user-provided options such as render width and height.
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
    /** @brief The number of samples per pixel. **/
    size_t samples;
};

/** @class KernelObject
  * @brief Uniform kernel argument interface
  * 
  * Each kernel object manages its own memory (both host-side and device-side)
  * and binds itself to a particular kernel argument slot, provided by the
  * engine. Inputs to kernel objects come in two forms:
  * - Scene data, where the kernel object requests the contents of a particular
  *   file located inside the selected scene's directory (such as geometry data
  *   or material definitions). The object loads this data itself.
  * - Engine parameters, which are always passed to the kernel object even if
  *   not specifically required. This contains parameters selected by the user
  *   at runtime, such as render width and height, or quality (samples/pixel).
  *
  * A kernel object can be passive, meaning it is responsible for storing data
  * and uploading it to the device once, or it can be active, meaning it will
  * be able to update itself and upload new data to the device as needed.
**/
class KernelObject
{
    protected:
        /** @brief The engine parameters, provided by the engine. **/
        const EngineParams& params;

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
          * @note No initialization should be performed in the constructor.
        **/
        KernelObject(const EngineParams& params) : params(params)
        {
            //this->params = 
        } 

        /** @brief Returns whether the kernel object is active.
          * @return If this returns \c true, then the kernel object's \c Update
          *         method will be called before each kernel invocation.
        **/
        virtual bool IsActive() = 0;

        /** @brief Initializes the kernel object.
        **/
        virtual void Initialize() = 0;

        /** @brief Binds the kernel object to a kernel.
          * @param kernel The OpenCL kernel to bind to.
          * @param slot The argument slot to bind to.
        **/
        virtual void Bind(cl::Kernel kernel, cl_uint slot) = 0;

        /** @brief Updates the kernel object.
          * @param index This indicates how many previous kernel invocations
          *              have occurred (e.g. if this contains 3, then this
          *              method has already been called three times).
          * @note In total, this method will be called \c params.samples times.
        **/
        virtual void Update(size_t index) = 0;

        /** @brief Returns object-specific information.
          * @param query A value identifying which information is required.
          * @returns A pointer to the requested information. The format of
          *          this information may vary and is assumed to be known
          *          to the caller. If the information is not found, returns
          *          \c nullptr (the null pointer).
          * @note Do not attempt to modify or deallocate the returned memory,
          *       as it belongs to the kernel object.
        **/
        virtual void* Query(size_t query) = 0;

        /** @brief Frees the kernel object.
        **/
        virtual ~KernelObject() { }
};
