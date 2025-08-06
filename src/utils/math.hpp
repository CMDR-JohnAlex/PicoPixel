#pragma once

#include <cmath>

namespace PicoPixel
{
    namespace Utils
    {
        struct Vec3
        {
            float x, y, z;

            Vec3()
             : x(0), y(0), z(0)
            {
            }
            Vec3(float xyz)
             : x(xyz), y(xyz), z(xyz)
            {
            }
            Vec3(float x, float y, float z)
             : x(x), y(y), z(z)
            {
            }
            ~Vec3() = default;

            Vec3 operator+(const Vec3& other) const
            {
                return Vec3(x + other.x, y + other.y, z + other.z);
            }
            Vec3 operator-(const Vec3& other) const
            {
                return Vec3(x - other.x, y - other.y, z - other.z);
            }
            Vec3 operator*(float scalar) const
            {
                return Vec3(x * scalar, y * scalar, z * scalar);
            }
        };

        struct Mat3
        {
            float m[3][3];

            Mat3()
            {
                Identity();
            }

            void Identity()
            {
                for(int i = 0; i < 3; i++)
                    for(int j = 0; j < 3; j++)
                        m[i][j] = (i == j) ? 1.0f : 0.0f;
            }

            void RotateX(float angle)
            {
                float cos_a = cosf(angle);
                float sin_a = sinf(angle);
                Identity();
                m[1][1] = cos_a;  m[1][2] = -sin_a;
                m[2][1] = sin_a;  m[2][2] = cos_a;
            }

            Vec3 Transform(const Vec3& v) const
            {
                return Vec3(
                    m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
                    m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
                    m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
                );
            }
        };
    }
}
