#pragma once
#include <common/common.hpp>

#define MAX_PLATFORMS 8
#define MAX_DEVICES   8

struct Device
{
    std::string name;
    cl_device_id ptr;
};

struct Platform
{
    std::string name;
    cl_platform_id ptr;
    std::vector<Device> devices;
};

/* This class simply enumerates OpenCL 1.2 compatible platforms/devices, on the
 * host system. It returns a readable string for display in the Interface class
 * and device pointers for the renderer to use upon initialization.          */
class DeviceList
{
    public:
        std::vector<Platform> platforms;
        void Initialize();
};
