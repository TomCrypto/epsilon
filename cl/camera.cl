/* Camera structure (device-side). */
typedef struct Camera
{
    float4 p[4];
    float4 pos;
	float4 up, left;
	float spread;
} Camera;

float3 lerp(float3 a, float3 b, float t)
{
    return a + (b - a) * t;
}

/* Traces a camera ray from a (u, v) normalized pixel coordinate. */
void Trace(float u, float v, float3 *origin, float3 *direction,
           constant Camera *camera, float u1, float u2)
{
    *origin = camera->pos.xyz;

	u2 = sqrt(u2);
	*origin = *origin + camera->up.xyz * cos(u1 * 2 * 3.14159265f) * u2 * camera->spread + camera->left.xyz * sin(u1 * 2 * 3.14159265f) * u2 * camera->spread;

    *direction = lerp(lerp(camera->p[0].xyz, camera->p[1].xyz, u),
                      lerp(camera->p[3].xyz, camera->p[2].xyz, u), v);
	*direction = normalize(*direction - *origin);
}
