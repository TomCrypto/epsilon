#pragma once

#include <engine/architecture.hpp>

/** @file prng.hpp
  * @brief Lightweight device PRNG.
**/

/** @class PRNG
  * @brief Pseudorandom number generator.
  *
  * This is a lightweight, cryptographic-grade pseudorandom number generator,
  * wrapped up as a kernel object. It uploads a new seed to the device at
  * every render pass (otherwise each pass would yield the same result).
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
        PRNG(EngineParams& params);
        ~PRNG() { }

        void Bind(cl_uint* index);
        void Update(size_t index);
        void* Query(size_t query);
};
