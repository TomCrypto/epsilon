 #pragma once

/** @file util.cl
  * @brief Kernel utilities.
  *
  * This file provides some utility functions and constants to simplify the
  * rest of the code.
**/

/** This is the ray-triangle epsilon value, used in intersection tests. **/
#define EPSILON 1e-5f

/** This is the pushback epsilon value, used to handle transmission/reflection,
  * It must be lower than \c EPSILON or artifacts will appear. **/
#define PSHBK 1e-4f

/** Computes the amplitude reflection coefficient for s-polarized light.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param cosI The cosine of the incident angle.
  * @param cosT The cosine of the transmitted angle.
**/
float rs(float n1, float n2, float cosI, float cosT)
{
    return (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
}

/** Computes the amplitude reflection coefficient for p-polarized light.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param cosI The cosine of the incident angle.
  * @param cosT The cosine of the transmitted angle.
**/ 
float rp(float n1, float n2, float cosI, float cosT)
{
    return (n2 * cosI - n1 * cosT) / (n1 * cosT + n2 * cosI);
}

/** Computes the amplitude transmission coefficient for s-polarized light.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param cosI The cosine of the incident angle.
  * @param cosT The cosine of the transmitted angle.
**/ 
float ts(float n1, float n2, float cosI, float cosT)
{
    return 2.0f * n1 * cosI / (n1 * cosI + n2 * cosT);
}

/** Computes the amplitude transmission coefficient for p-polarized light.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param cosI The cosine of the incident angle.
  * @param cosT The cosine of the transmitted angle.
**/
float tp(float n1, float n2, float cosI, float cosT)
{
    return 2.0f * n1 * cosI / (n1 * cosT + n2 * cosI);
}

/** Computes the Fresnel power reflection coefficient at an interface between
  * two media, given an incident vector and a normal vector.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param incident The incident vector.
  * @param normal The normal vector.
  * @returns The reflection coefficient, between 0 and 1.
  * @note The transmission coefficient is equal to one minus the reflection
  *       coefficient, due to conservation of energy.
**/
float Fresnel(float n1, float n2, float3 incident, float3 normal)
{
    /* Compute the reflection and refraction angles via Snell's Law. */
    float cosI = fabs(dot(incident, normal));
    float cosT = sqrt(1.0f - pow(n1 / n2, 2) * (1.0f - pow(cosI, 2)));

    /* Compute the Fresnel amplitude terms for both polarizations. */
    float s = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float p = (n1 * cosT - n2 * cosI) / (n1 * cosT + n2 * cosI);

    /* Compute the reflection intensity for nonpolarized light. */
    return (pow(s, 2) + pow(p, 2)) * 0.5f;
}

/** This function is the same as \c Fresnel, except it takes incident and
  * transmission angles, and is thus faster if these are already available.
  * @param n1 The incoming medium's refractive index.
  * @param n2 The outgoing medium's refractive index.
  * @param cosI The cosine of the incident angle.
  * @param cosT The cosine of the transmitted angle.
  * @returns The reflection coefficient, between 0 and 1.
**/
float FresnelCos(float n1, float n2, float cosI, float cosT)
{
    /* Compute the Fresnel amplitude terms for both polarizations. */
    float s = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float p = (n1 * cosT - n2 * cosI) / (n1 * cosT + n2 * cosI);

    /* Compute the reflection intensity for nonpolarized light. */
    return (pow(s, 2) + pow(p, 2)) * 0.5f;
}

/** Linearly interpolates between two vectors.
  * @param a The "start" vector.
  * @param b The "end" vector.
  * @param t The interpolation parameter.
  * @returns The linearly interpolated vector.
**/
float3 lerp(float3 a, float3 b, float t)
{
    return a + (b - a) * t;
}
