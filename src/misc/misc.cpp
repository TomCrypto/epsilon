#include <misc/misc.hpp>

Progress::Progress(EngineParams& params) : KernelObject(params)
{
    return;
}

void Progress::Bind(cl_uint* /* index */)
{
    return;
}

void Progress::Update(size_t pass)
{
    this->progress = (double)pass / params.passes;

    if (pass == 0) this->startTime = time(nullptr);

    this->elapsed = difftime(time(nullptr), this->startTime);
    if (this->elapsed < 5)
    {
        this->ETC = -1.0; /* Indeterminate. */
    }
    else
    {
        this->ETC = this->elapsed * (double)(params.passes - pass) / pass;
    }
}

void* Progress::Query(size_t query)
{
    if (query == Query::Progress) return &this->progress;
    if (query == Query::EstimatedTime) return &this->ETC;
    if (query == Query::ElapsedTime) return &this->elapsed;
    return nullptr;
}
