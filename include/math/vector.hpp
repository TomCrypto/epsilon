#pragma once

/* This file contains definitions for a three-dimensional vector to assist in *
 * performing vector math which can be exported to the device cl_float4 type. */

#include <common/common.hpp>

struct Vector
{
    float x, y, z;

    Vector() { x = y = z = 0; }
    Vector (float x, float y, float z) : x(x), y(y), z(z) { }

    void operator += (Vector a) { *this = *this + a; }
    void operator -= (Vector a) { *this = *this - a; }
    void operator *= (Vector a) { *this = *this * a; }
    void operator /= (Vector a) { *this = *this / a; }
    void operator *= (float a)  { *this = *this * a; }
    void operator /= (float a)  { *this = *this / a; }

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

    void CL(cl_float4 *out)
    {
        out->s[0] = this->x;
        out->s[1] = this->y;
        out->s[2] = this->z;
        out->s[3] = 1.0f;
    }

    friend std::ostream& operator << (std::ostream& out, const Vector& v);
};

float dot(const Vector& a, const Vector& b);

Vector cross(const Vector& a, const Vector& b);

float length(const Vector& a);

Vector normalize(const Vector& a);

Vector vmin(const Vector& a, const Vector& b);
Vector vmax(const Vector& a, const Vector& b);

void Basis(const Vector& bitangent, Vector *normal, Vector *tangent);

Vector Transform(const Vector& in, const Vector& tangent,
                                   const Vector& normal,
                                   const Vector& bitangent);
