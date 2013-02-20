#include <misc/etc.hpp>

bool ETC::IsActive() { return true; }

void ETC::Initialize(const EngineParams& params)
{
    return;
}

void ETC::Bind(cl::Kernel kernel, cl_uint slot)
{
    return;
}

void ETC::Update(const EngineParams& params, size_t index)
{
    if (index == 0) startTime = time(nullptr);
    size_t currentTime = time(nullptr);
    if (currentTime - startTime < 5)
    {
        this->etc = (size_t)-1; /* Indeterminate. */
    }
    else
    {
        /* index samples in cur - start seconds, so 1 sample in:
         * (cur - start) / index -> */
        double d = currentTime - startTime;
        this->etc = (double)(params.samples - index) / index * d;
    }


    return;
}

void* ETC::Query(const EngineParams& params, size_t query)
{
    if (query == Query::EstimatedTime)
    {
        return &this->etc;
    }

    return nullptr;
}

void ETC::Cleanup(const EngineParams& params)
{
    return;
}
