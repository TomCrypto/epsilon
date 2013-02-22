#pragma once

#include <common/common.hpp>
#include <engine/architecture.hpp>

/** @file prng.hpp
  * @brief Lightweight device PRNG.
**/

/** @class PRNG
  * @brief Pseudorandom number generator.
  *
  * This is a lightweight, cryptographic-grade pseudorandom number generator,
  * wrapped up as a kernel object. It is active, as it uploads a new seed to
  * the device at every kernel invocation.
  *
  * This kernel object handles no queries.
**/
class PRNG : public KernelObject
{
    private:
        /** @brief We use a 64-bit seed though we could go up to 256 bits. **/
        uint64_t seed;

        /** @brief This is the device-side buffer containing the seed. **/
        cl::Buffer buffer;
    public:
		PRNG(EngineParams& params) : KernelObject(params) { }
		~PRNG() { }

        bool IsActive();
        void Initialize();
        void Bind(cl_uint* slot);
        void Update(size_t index);
        void* Query(size_t query);
};
