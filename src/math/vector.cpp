#include <math/vector.hpp>

float dot(const Vector& a, const Vector& b)
{
    return a.x * b.x
         + a.y * b.y
         + a.z * b.z;
}

Vector cross(const Vector& a, const Vector& b)
{
    return Vector(a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}

float length(const Vector& a)
{
    return sqrt(dot(a, a));
}

Vector normalize(const Vector& a)
{
    return a / length(a);
}

Vector vmin(const Vector& a, const Vector& b)
{
    return Vector(std::min(a.x, b.x),
                  std::min(a.y, b.y),
                  std::min(a.z, b.z));
}

Vector vmax(const Vector& a, const Vector& b)
{
    return Vector(std::max(a.x, b.x),
                  std::max(a.y, b.y),
                  std::max(a.z, b.z));
}

void Basis(const Vector& bitangent, Vector *normal, Vector *tangent)
{
    *tangent = normalize(cross(bitangent, Vector(0.0f, 1.0f, 0.0f)));
    *normal = normalize(cross(*tangent, bitangent));
}

Vector Transform(const Vector& in, const Vector& tangent,
                                   const Vector& normal,
                                   const Vector& bitangent)
{
    return tangent * in.x + normal * in.y + bitangent * in.z;
}
