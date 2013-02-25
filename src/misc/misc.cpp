#include <misc/misc.hpp>

Progress::Progress(EngineParams& params) : KernelObject(params)
{
    this->progress = 0;
    this->elapsed = 0.0;
    this->remains = -1.0;
}

void Progress::Bind(cl_uint* /* index */)
{
	timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	startTime = time.tv_sec + time.tv_nsec * 1e-9;
}

void Progress::Update(size_t pass)
{
    this->progress = (double)(pass + 1) / params.passes;

	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	double time = now.tv_sec + now.tv_nsec * 1e-9;

    this->elapsed = time - startTime;
    if (this->elapsed < 5)
    {
        this->remains = -1.0; /* Indeterminate. */
    }
    else
    {
        double position = (double)(params.passes - pass - 1) / (pass + 1);
        this->remains = this->elapsed * position;
    }
}

void* Progress::Query(size_t query)
{
    if (query == Query::Progress) return &this->progress;
    if (query == Query::EstimatedTime) return &this->remains;
    if (query == Query::ElapsedTime) return &this->elapsed;
    return nullptr;
}
