#pragma once

#include <util.cl>
#include <prng.cl>

/** @file camera.cl
  * @brief Kernel camera implementation.
**/

/** @struct Camera
  * @brief Camera parameters.
**/
typedef struct Camera
{
    /** The focal plane, as a quad. **/
    float4 p[4];
    /** The camera position. **/
    float3 pos;
    /** The camera's "up" vector. **/
    float3 up;
    /** The camera's "left" vector. **/
    float3 left;
    /** The camera's focal spread (aperture radius). **/
    float spread;
} Camera;

/** Computes a camera ray for a given pixel.
  * @param u The normalized x-coordinate.
  * @param v The normalized y-coordinate.
  * @param origin A pointer to the camera ray's origin.
  * @param direction A pointer to the camera ray's direction, as a unit vector.
  * @param x A uniform in [0..1).
  * @param y Another uniform in [0..1).
  * @param camera The camera parameters.
**/
void Trace(float u, float v, float3 *origin, float3 *direction,
           float x, float y, constant Camera *camera)
{
    *origin = camera->pos.xyz;

    *origin += (camera->up.xyz   * cos(x * 2 * PI)
             +  camera->left.xyz * sin(x * 2 * PI))
             * sqrt(y) * camera->spread;

    *direction = lerp(lerp(camera->p[0].xyz, camera->p[1].xyz, u),
                      lerp(camera->p[3].xyz, camera->p[2].xyz, u), v);

    *direction = normalize(*direction - *origin);
}
