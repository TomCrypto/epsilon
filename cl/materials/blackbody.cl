/* Black-body emission spectrum, any temperature. */
float blackbody(float wavelength, float temperature)
{
    float powerTerm = 3.74183e-16f * pow(wavelength, -5.0f);
    return powerTerm / (exp(1.4388e-2f / (wavelength * temperature)) - 1.0f);
}
