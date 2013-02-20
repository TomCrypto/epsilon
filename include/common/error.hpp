#pragma once

#include <common/common.hpp>

/** @file error.hpp
  * @brief Error code declarations.
**/

/** @namespace Error
  *
  * @brief Contains all possible error messages and error handling methods.
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
	/** @brief The CLC build log (if build fails) could not be retrieved. **/
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

/* This file contains some error definitions. */
#define E_PLATFM "Failed to query OpenCL platforms!"
#define E_DEVICE "Failed to query OpenCL devices!"
#define E_CTX "Failed to create context!"
#define E_QUEUE "Failed to create queue!"
#define E_BUF "Failed to create buffer!"
#define E_READ "Failed to read from buffer!"
#define E_WRITE "Failed to write to buffer!"
#define E_BIND "Failed to bind buffer to kernel!"
#define E_INFO "Failed to acquire device information!"
#define E_PROG "Failed to create program!"
#define E_BUILD "Failed to build program! Compilation log available."
#define E_KER "Failed to create kernel!"

