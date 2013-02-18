#include <devices/devices.hpp>

/* Trims a string. */
void trim(char *s)
{
    char * p = s;
    int l = strlen(p);
    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;
    memmove(s, p, l + 1);
}

void DeviceList::Initialize()
{
    cl_uint count = 0;
    cl_platform_id p_list[MAX_PLATFORMS];
    cl_int error = clGetPlatformIDs(MAX_PLATFORMS, p_list, &count);

    if (count == 0) throw std::runtime_error("No OpenCL platform available.");
    if (error != CL_SUCCESS) throw std::runtime_error(Error(E_PLATFM, error));

    this->platforms.resize(count);
    for (size_t t = 0; t < count; ++t)
    {
        this->platforms[t].ptr = p_list[t];

        char name[1024];
        clGetPlatformInfo(p_list[t], CL_PLATFORM_NAME, sizeof(name), name, 0);
        trim(name);
        this->platforms[t].name = std::string(name);

        cl_uint devices = 0;
        cl_device_id d_list[MAX_DEVICES];
        error = clGetDeviceIDs(p_list[t], CL_DEVICE_TYPE_ALL, MAX_DEVICES,
                               d_list, &devices);

        if (error != CL_SUCCESS)
        {
            throw std::runtime_error(Error(E_DEVICE, error));
        }

        this->platforms[t].devices.resize(devices);
        for (size_t j = 0; j < devices; ++j)
        {
            this->platforms[t].devices[j].ptr = d_list[j];
            clGetDeviceInfo(d_list[j], CL_DEVICE_NAME,
                              sizeof(name), name, 0);

            trim(name);
            this->platforms[t].devices[j].name = std::string(name);
        }
    }
}
