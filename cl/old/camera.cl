/* Camera structure (device-side). */
typedef struct Camera
{
    float4 p[4];
    float4 pos;
} Camera;

float3 lerp(float3 a, float3 b, float t)
{
    return a + (b - a) * t;
}

/* Traces a camera ray from a (u, v) normalized pixel coordinate. */
void Trace(float u, float v, float3 *origin, float3 *direction,
           constant Camera *camera)
{
    *origin = camera->pos.xyz;
    *direction = lerp(lerp(camera->p[0].xyz, camera->p[1].xyz, u),
                      lerp(camera->p[3].xyz, camera->p[2].xyz, u), v);
    *direction = normalize(*direction - *origin);  
}
