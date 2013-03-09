#pragma once

#include <prng.cl>
#include <util.cl>

/** @file material.cl
  * @brief Material dispatching.
  *
  * This file is responsible for dispatching material ID's to the appropriate
  * handling code. Each material ID has its own OpenCL file, this is just a
  * giant switch statement. You are meant to add your own material ID's so
  * feel free to edit this file.
  *
  * \todo Actually separate different materials in different files. This is
  *       for testing.
**/

/** @brief Ambient atmosphere (note \c reflect is never called with this).
  * @note This material must never be used, as it maintains an invariant. **/
#define ATMOSPHERE 0x00000000

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

/** This function returns the material's exitant spectral radiance, this is the
  * lighting term, if the material is not a light source this must be negative.
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
    if (matID == WHITE_FLUORESCENT)
    {
        float powerTerm = 3.74183e-16f * pow(wavelength, -5.0f);
        return powerTerm / (exp(1.4388e-2f / (wavelength * 3500)) - 1.0f);
    }

    return -1; /* Not a light source. */
}

/** This function returns the material's absorption coefficient.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @return The absorption coefficient.
**/
float absorption(uint matID, float wavelength)
{
    if (matID == GLASS_RED)
    {
        float w = wavelength * 1e9;

        return 1e-5 + 5.2 * (1 - exp(-pow(w - 660, 2) * 0.001f));
    }

    return 1e-5; /* No absorption for now. */
}

/** This function returns the material's refractive index.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @returns The material's refractive index at the given wavelength.
**/
float index(uint matID, float wavelength)
{
    if (matID == GLASS_RED) return 1.55f;

    return 1.0f; /* Air.. sort of. */
}

/** This function evaluates the material's phase function and returns an
  * importance-sampled scattered ray.
  * @param matID The material ID.
  * @param wavelength The light's wavelength.
  * @param prng A PRNG instance.
  * @returns An importance-sampled scattered ray, in unit space. Rotate
  *          according to the incident ray to obtain the scattered ray
  *          in world space.
  * @note The w-component of the resulting ray contains the normalization term
  *       in case the phase function does not integrate to 1, this value should
  *       be multiplied with the radiance.
**/
float4 scatter(uint matID, float wavelength, PRNG *prng)
{
    if (matID == GLASS_RED)
    {
        /* Ray decay, as it got absorbed. */
        return (float4)(0, 1, 0, 0.0);
    }

    return (float4)(0, 0, 0, 0); /* No scattering for now. */
}

/** This function evaluates the material's reflectance function and returns an
  * importance-sampled reflected or transmitted ray.
  * @param in The material ID of the medium the ray is in.
  * @param to The material ID of the medium beyond the interface.
  * @param wavelength The light's wavelength.
  * @param incident The incident ray, in TBN space.
  * @param prng a PRNG instance.
  * @returns An importance-sampled reflected or transmitted ray, in TBN space.
  *          Rotate via the TBN basis to obtain the ray in world space.
  * @note To verify if the ray was transmitted or not, it is enough to check
  *       the sign of \c dot(result, <0, 1, 0>).
  * @note The w-component of the resulting ray contains the normalization term
  *       in case the reflectance function does not integrate to 1, this value 
  *       should be multiplied with the radiance.

**/
float4 reflect(uint in, uint to, float wavelength, float3 incident, PRNG *prng)
{
    if ((to == GLASS_RED) || (in == GLASS_RED))
    {
        float3 direction;

        float n1 = index(in, wavelength);
        float n2 = index(to, wavelength);

        float cosI = -dot(incident, (float3)(0, 1, 0));
        float cosT = 1.0f - pow(n1 / n2, 2) * (1.0f - pow(cosI, 2));

        if (cosT < 0)
        {
            direction = incident - 2 * (float3)(0, 1, 0) * cosI;
        }
        else
        {
            cosT = sqrt(cosT);

            float R = Fresnel(n1, n2, incident, (float3)(0, 1, 0));

            if (rand(prng) < R)
            {
                direction = incident - 2 * (float3)(0, 1, 0) * cosI;
            }
            else
            {
                direction = incident * (n1 / n2) + (float3)(0, 1, 0) * ((n1 /
                n2) * cosI - cosT);
            }
        }

        return (float4)(direction, 0.9f);
    }

    float peak = 0.0f;

    switch(to)
    {
        case DIFFUSE_GREEN: peak = 525; break;
        case DIFFUSE_BLUE: peak = 460; break;
        case DIFFUSE_RED: peak = 640; break;
    }

    float w = wavelength * 1e9;

    float response = (peak == 0) ? 1.0 : 0.45 + exp(-pow(w - peak, 2) * 0.001f)
    * 0.55;

    response *= 0.8;

    float u1 = rand(prng);
    float u2 = rand(prng);
    float phi = 2 * 3.14159265f * u2;
    float r = sqrt(u1);

    float3 reflected = (float3)(r * cos(phi), sqrt(1.0f - u1), r *
    sin(phi));

    return (float4)(reflected, response);
}
