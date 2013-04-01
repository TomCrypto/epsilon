#pragma once

#include <prng.cl>
#include <util.cl>

#include <materials/glass.cl>
#include <materials/matte.cl>
#include <materials/blackbody.cl>

/** @file material.cl
  * @brief Material dispatching.
  *
  * This file is responsible for dispatching material ID's to the appropriate
  * handling code. Each material ID has its own OpenCL file, this is just one
  * giant switch statement. You are meant to add your own materials, so, feel
  * free to edit this file.
  *
  * \todo Finalize the material system's interface.
**/

/** @brief Vacuum, used as an atmosphere for non-volumetric renders. **/
#define VACUUM 0x00000000

/** @brief White fluorescent light source (~13 Watt). **/
#define WHITE_FLUORESCENT 0x00000001

/** @brief Colorful diffuse green wall. **/
#define DIFFUSE_GREEN 0x00000002

/** @brief Colorful diffuse red wall. **/
#define DIFFUSE_RED 0x00000003

/** @brief Colorful diffuse blue wall. **/
#define DIFFUSE_BLUE 0x00000004

/** @brief White diffuse wall. **/
#define DIFFUSE_WHITE 0x00000005

/** @brief Absorbing red glass material. **/
#define GLASS_RED 0x00000006

/** @brief Absorbing red glass material (weaker). **/
#define GLASS_RED_WEAK 0x00000007

/** This function returns the material's exitant spectral radiance, this is the
  * lighting term, if the material is not a light source this must be negative,
  * or zero, otherwise the material will be considered as emissive.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @param incident The incident ray, in TBN space.
  * @param prng A PRNG instance.
  * @returns The exitant spectral radiance.
  * @note If this returns a positive result, then the material ID will never be
  *       used in any other function (\c absorption, \c scatter, \c reflect).
**/
float exitant(uint matID, float wavelength, float3 incident, PRNG *prng)
{
    /* Because the default behavior is to return negative 1, if the material *
     * is not emissive (not a light source) you are free to ignore this.     */
    switch (matID)
    {
        case WHITE_FLUORESCENT: return blackbody(wavelength, 3500.0f);

        default: return -1.0f;
    }
}

/** This function returns the material's absorption coefficient.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @return The absorption coefficient.
**/
float absorption(uint matID, float wavelength)
{
    /* The default behavior of this function is to return a value very close *
     * to zero, corresponding to no absorption. If the material doesn't have *
     * absorption properties, you are free to not implement this function.   */ 
    switch (matID)
    {
        case GLASS_RED: return glass_abs(wavelength, 1.2f);
        case GLASS_RED_WEAK: return glass_abs(wavelength, 0.08f);

        default: return 1e-6f;
    }
}

/** This function returns the material's refractive index.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @returns The material's refractive index at the given wavelength.
**/
float index(uint matID, float wavelength)
{
    /* The default behavior of this function is to explicitly return zero to *
     * prevent forgetting to implement it, as it is required: any medium has *
     * a refractive index, and there is no reasonable "default value".       */
    switch (matID)
    {
        case VACUUM: return 1.0f;
        case GLASS_RED: return 1.55f;
        case GLASS_RED_WEAK: return 1.55f;

        default: return -1.0f;
    }
}

/** This function evaluates the material's phase function and returns an
  * importance-sampled scattered ray.
  * @param matID The material ID.
  * @param w The light's wavelength.
  * @param prng A PRNG instance.
  * @returns An importance-sampled scattered ray, in unit space. Rotate
  *          according to the incident ray to obtain the scattered ray
  *          in world space.
  * @note The w-component of the resulting ray contains the normalization term
  *       in case the phase function does not integrate to 1 - which should be
  *       multiplied with the radiance.
**/
float4 scatter(uint matID, float w, PRNG *prng)
{
    /* For materials which are purely absorbing, we assume scattering causes *
     * the ray intensity to decay to nil. Thus if your material doesn't have *
     * this property, you don't need to implement the scatter function.      */
    switch (matID)
    {
        default: return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

/** This function evaluates the material's reflectance function and returns an
  * importance-sampled reflected or transmitted ray.
  * @param in The material ID of the medium the ray is in.
  * @param to The material ID of the medium beyond the interface.
  * @param w The light's wavelength.
  * @param incident The incident ray, in TBN space.
  * @param prng a PRNG instance.
  * @param nested Whether the \c to medium is nested inside the \c in medium.
  * @returns An importance-sampled reflected or transmitted ray, in TBN space.
  *          Rotate via the TBN basis to obtain the ray in world space.
  * @note To verify if the ray was transmitted or not, it is enough to check
  *       the sign of \c result.y. If negative, the ray was transmitted.
  * @note The w-component of the resulting ray contains the normalization term
  *       in case the reflectance function does not integrate to 1, this value 
  *       should be multiplied with the radiance.

**/
float4 reflect(uint in, uint to, float w, float3 incident, PRNG *prng,
               bool nested)
{
    switch (nested? to : in)
    {
        case DIFFUSE_WHITE: return matte_flat(w, prng, 0.8f);
        case DIFFUSE_RED: return matte_peak(w, prng, 640, 0.001f, 0.55f);
        case DIFFUSE_BLUE: return matte_peak(w, prng, 460, 0.001f, 0.55f);
        case DIFFUSE_GREEN: return matte_peak(w, prng, 525, 0.01f, 0.55f);
        case GLASS_RED:
        {
            float n1 = index(in, w);
            float n2 = index(to, w);

            return glass(prng, incident, n1, n2, 0.9f);
        }
        case GLASS_RED_WEAK:
        {
            float n1 = index(in, w);
            float n2 = index(to, w);

            return glass(prng, incident, n1, n2, 0.9f);
        }

        default: return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    }
}
