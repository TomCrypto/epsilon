#pragma once

#include <algorithm>
#include <CL/cl.hpp>
#include <cmath>

/** @file vector.hpp
  * @brief Vector manipulation.
  *
  * This file contains methods to perform vector operations, as well as a few
  * useful mathematical constants and functions.
**/

/** @brief The ratio of a circle's circumference to its radius. **/
#define PI 3.14159265f

/** @brief A small value used in floating-point comparisons for stability. **/
#define EPSILON 1e-5f

/** @brief Returns the sign of \c x, as (\c -1) or (\c +1). **/
#define sign(x) ((x > 0.0) - (x < 0.0))

/** @struct Vector
  * @brief Device-side vector.
  *
  * This is a convenient implementation of the device-side \c cl_float4 type,
  * to facilitate mathematical operations on the host system.
**/
struct Vector
{
    float x, y, z;

    /** @brief Constructs a default vector with all components zero.
    **/
    Vector() { x = y = z = 0; }

    /** @brief Constructs a vector with the passed components.
      * @param x The x-coordinate (left-right)
      * @param y The y-coordinate (up-down)
      * @param z The z-coordinate (in-out)
    **/
    Vector (float x, float y, float z) : x(x), y(y), z(z) { }

    void operator += (Vector a) { *this = *this + a; }
    void operator -= (Vector a) { *this = *this - a; }
    void operator *= (Vector a) { *this = *this * a; }
    void operator /= (Vector a) { *this = *this / a; }
    void operator *= (float a)  { *this = *this * a; }
    void operator /= (float a)  { *this = *this / a; }

    /** @brief Returns one component of the vector.
      * @param index The index of the component to return.
      * @return This interprets the vector components as an array of three
      *         elements, as [\c x, \c y, \c z].
    **/
	float operator [] (int index)
	{
		if (index == 0) return x;
		if (index == 1) return y;
		if (index == 2) return z;
		return 0.0f;
	}

    Vector operator + (const Vector& b) const
    {
        return Vector(this->x + b.x,
                      this->y + b.y,
                      this->z + b.z);
    }

    Vector operator - (const Vector& b) const
    {
        return Vector(this->x - b.x,
                      this->y - b.y,
                      this->z - b.z);
    }

    Vector operator * (const Vector& b) const
    {
        return Vector(this->x * b.x,
                      this->y * b.y,
                      this->z * b.z);
    }

    Vector operator / (const Vector& b) const
    {
        return Vector(this->x / b.x,
                      this->y / b.y,
                      this->z / b.z);
    }

    Vector operator * (float b) const
    {
        return Vector(this->x * b,
                      this->y * b,
                      this->z * b);
    }

    Vector operator / (float b) const
    {
        return (*this) * (1.0f / b);
    }

    /** @brief This method converts the vector to the device type \c cl_float4.
      * @param out A pointer to the \c cl_float4 instance to write to.
    **/
    void CL(cl_float4 *out)
    {
        out->s[0] = this->x;
        out->s[1] = this->y;
        out->s[2] = this->z;
        out->s[3] = 1.0f;
    }
};

/** @brief Returns the dot product of two vectors.
  * @param a The first vector.
  * @param b The second vector.
**/
float dot(const Vector& a, const Vector& b);

/** @brief Returns the cross product of two vectors.
  * @param a The first vector.
  * @param b The second vector.
  * @note This operation is not commutative.
**/
Vector cross(const Vector& a, const Vector& b);

/** @brief Returns the length of a vector.
  * @param a The vector in question.
  * @note For unit vectors, will return \c 1 (to floating-point accuracy).
**/
float length(const Vector& a);

/** @brief Normalizes a vector.
  * @param a The vector to normalize.
  * @return The returned vector will have the same direction as \c a, but a
  *       length of \c 1.
**/
Vector normalize(const Vector& a);

/** @brief Returns the component-wise minimum of two vectors.
  * @param a The first vector.
  * @param b The second vector.
  * @return Each component of the returned vector will be the minimum component
  *       of \c a and \c b (i.e. the x-coordinate of the returned vector will
  *       be equal to \c min(\c a.x, \c b.x), and so on).
**/
Vector vmin(const Vector& a, const Vector& b);

/** @brief Returns the component-wise maximum of two vectors.
  * @param a The first vector.
  * @param b The second vector.
  * @return Each component of the returned vector will be the maximum component
  *       of \c a and \c b (i.e. the x-coordinate of the returned vector will
  *       be equal to \c max(\c a.x, \c b.x), and so on).
**/
Vector vmax(const Vector& a, const Vector& b);

/** @brief Constructs an orthogonal basis (tangent/normal/bitangent).
  * @param bitangent The vector which is to be the bitangent of the basis.
  * @param normal A pointer to the vector to write the basis normal in.
  * @param tangent A pointer to the vector to write the basis tangent in.
**/
void Basis(const Vector& bitangent, Vector *normal, Vector *tangent);

/** @brief Transforms a vector into a given orthonormal basis.
  * @param in The vector to transform.
  * @param tangent The basis tangent.
  * @param normal The basis normal.
  * @param bitangent The basis bitangent.
  * @return The transformed vector.
**/
Vector Transform(const Vector& in, const Vector& tangent,
                                   const Vector& normal,
                                   const Vector& bitangent);
