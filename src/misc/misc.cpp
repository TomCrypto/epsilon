#include <misc/misc.hpp>

Progress::Progress(EngineParams& params) : KernelObject(params)
{
    this->progress = 0;
    this->elapsed = 0.0;
    this->ETC = -1.0;
}

void Progress::Bind(cl_uint* /* index */)
{
    this->startTime = time(nullptr);
    return;
}

void Progress::Update(size_t pass)
{
    this->progress = (double)(pass + 1) / params.passes;

    this->elapsed = difftime(time(nullptr), this->startTime);
    if (this->elapsed < 5)
    {
        this->ETC = -1.0; /* Indeterminate. */
    }
    else
    {
        double position = (double)(params.passes - pass - 1) / (pass + 1);
        this->ETC = this->elapsed * position;
    }
}

void* Progress::Query(size_t query)
{
    if (query == Query::Progress) return &this->progress;
    if (query == Query::EstimatedTime) return &this->ETC;
    if (query == Query::ElapsedTime) return &this->elapsed;
    return nullptr;
}
