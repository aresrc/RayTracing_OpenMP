#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>

struct Vector3 {
    double x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3 &o) const { return Vector3(x+o.x, y+o.y, z+o.z); }
    Vector3 operator-(const Vector3 &o) const { return Vector3(x-o.x, y-o.y, z-o.z); }

    // NUEVO — multiplicación componente a componente
    Vector3 operator*(const Vector3 &o) const {
        return Vector3(x * o.x, y * o.y, z * o.z);
    }

    // Ya existía — multiplicación por escalar
    Vector3 operator*(double s) const { return Vector3(x*s, y*s, z*s); }

    Vector3 operator/(double s) const { return Vector3(x/s, y/s, z/s); }

    Vector3& operator+=(const Vector3 &o) { x+=o.x; y+=o.y; z+=o.z; return *this; }

    double dot(const Vector3 &o) const { return x*o.x + y*o.y + z*o.z; }

    Vector3 cross(const Vector3 &o) const {
        return Vector3(
            y*o.z - z*o.y,
            z*o.x - x*o.z,
            x*o.y - y*o.x
        );
    }

    double length() const { return std::sqrt(x*x + y*y + z*z); }
    Vector3 normalized() const {
        double l = length();
        return (l == 0) ? Vector3(0,0,0) : (*this)/l;
    }
};

#endif
