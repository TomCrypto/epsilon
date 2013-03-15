#include <util.cl>

float glass_abs(float wavelength)
{
    float w = wavelength * 1e9;
    return 1e-5 + 1.2 * (1 - exp(-pow(w - 650, 2) * 0.001f));
}

/* Smooth glass material. */
float4 glass(PRNG* prng, float3 incident, float n1, float n2, float loss)
{
    float3 direction;
    float cosI = fabs(dot(incident, (float3)(0, 1, 0)));
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
            direction = incident * (n1 / n2)
                      + (float3)(0, 1, 0) * ((n1 / n2) * cosI - cosT);
        }
    }

    return (float4)(direction, loss);
}
