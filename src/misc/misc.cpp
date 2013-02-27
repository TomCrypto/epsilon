#include <misc/misc.hpp>

#include <ctime>

Progress::Progress(EngineParams& params) : KernelObject(params)
{
    this->progress = 0;
    this->elapsed = 0.0;
    this->remains = -1.0;
}

void Progress::Bind(cl_uint* /* index */)
{
    #ifdef LOW_RES_TIME
    startTime = time(nullptr);
    #else
	timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	startTime = time.tv_sec + time.tv_nsec * 1e-9;
    #endif
}

void Progress::Update(size_t pass)
{
    this->progress = (double)(pass + 1) / params.passes;

    #ifdef LOW_RES_TIME
    time_t now = time(nullptr);
    #else
	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	double time = now.tv_sec + now.tv_nsec * 1e-9;
    #endif

    #ifdef LOW_RES_TIME
    elapsed = difftime(now, startTime);
    #else
    elapsed = time - startTime;
    #endif

    if (elapsed < 5)
    {
        remains = -1.0; /* Indeterminate. */
    }
    else
    {
        double position = (double)(params.passes - pass - 1) / (pass + 1);
        remains = elapsed * position;
    }
}

void* Progress::Query(size_t query)
{
    if (query == Query::Progress) return &this->progress;
    if (query == Query::EstimatedTime) return &this->remains;
    if (query == Query::ElapsedTime) return &this->elapsed;
    return nullptr;
}
