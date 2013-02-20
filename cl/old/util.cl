/* Utility methods and constants. */

#define EPSILON 1e-6f

/* Fresnel equations - takes as a parameter a surface normal, and two indices
 * of refraction, n1 and n2, where n1 is the source medium, as well as an
 * incident ray. It returns the probability of the ray being reflected. */
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

float Fresnel2(float n1, float n2, float cosI, float cosT)
{
    /* Compute the Fresnel amplitude terms for both polarizations. */
    float s = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float p = (n1 * cosT - n2 * cosI) / (n1 * cosT + n2 * cosI);

    /* Compute the reflection intensity for nonpolarized light. */
    return (pow(s, 2) + pow(p, 2)) * 0.5f;
}

float rs(float n1, float n2, float cosI, float cosT)
{
    return (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
}

float rp(float n1, float n2, float cosI, float cosT)
{
    return (n2 * cosI - n1 * cosT) / (n1 * cosT + n2 * cosI);
}

float ts(float n1, float n2, float cosI, float cosT)
{
    return 2.0f * n1 * cosI / (n1 * cosI + n2 * cosT);
}

float tp(float n1, float n2, float cosI, float cosT)
{
    return 2.0f * n1 * cosI / (n1 * cosT + n2 * cosI);
}
