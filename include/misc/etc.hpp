#pragma once

#include <common/common.hpp>
#include <engine/architecture.hpp>

class ETC : public KernelObject
{
    private:
        size_t startTime;
        size_t etc;

    public:
        bool IsActive();
        void Initialize(const EngineParams& params);
        void Bind(cl::Kernel kernel, cl_uint slot);
        void Update(const EngineParams& params, size_t index);
        void* Query(const EngineParams& params, size_t query);
        void Cleanup(const EngineParams& params);
};
