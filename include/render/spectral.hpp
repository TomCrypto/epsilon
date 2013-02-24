#pragma once

#include <CL/cl.hpp>
#include <cstddef>

/** @file spectral.hpp
  * @brief Spectral data.
**/

/** @namespace Spectral
  * @brief Color-matching curve.
  *
  * This namespace contains the color-matching curve which associates a given
  * wavelength (e.g. 540nm) with the color perceived by a human observer, in
  * XYZ (perceptual) format.
**/
namespace Spectral
{
    struct XYZ
    {
        cl_float4 data;
        XYZ(float x, float y, float z)
        {
            data.s[0] = x;
            data.s[1] = y;
            data.s[2] = z;
            data.s[3] = 1.0f; /* By convention. */
        }
    };

    /** @brief The number of associated wavelengths.
      * @note This is the number of wavelengths between 380nm and 780nm (the
      *       visible spectrum) that have an associated XYZ color. The higher
      *       this number, the better the spectral rendering will be, though
      *       because missing wavelengths are linearly interpolated, a value
      *       of 81 (corresponding to 5nm intervals) is more than sufficient.
    **/
    size_t Resolution();

    /** @brief This returns the wavelength to XYZ curve, as an array.
    **/
    XYZ* Curve();
}
