/* Diffuse reflection, flat spectrum response. */
float4 matte_flat(float w, PRNG *prng, float albedo)
{
    float u = rand(prng), v = rand(prng);
    float a = 2 * 3.14169265f * v;
    float r = sqrt(u);

    float3 reflected = (float3)(r * cos(a), sqrt(1.0f - u), r * sin(a));
    return (float4)(reflected, albedo);   
}

/* Diffuse reflection, peak spectrum response <peak, deviation>. */
float4 matte_peak(float w, PRNG *prng, float peak, float dev, float albedo)
{
    float u = rand(prng), v = rand(prng);
    float a = 2 * 3.14169265f * v;
    float r = sqrt(u);
    w *= 1e9f;

    float3 reflected = (float3)(r * cos(a), sqrt(1.0f - u), r * sin(a));
    float intensity = exp(-pow(w - peak, 2) * dev);
    intensity = albedo * intensity + (1 - albedo);
    return (float4)(reflected, intensity);
}

