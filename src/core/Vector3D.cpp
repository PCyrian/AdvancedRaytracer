/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/core/Point3D.hpp"

Math::Vector3D::Vector3D() : x(0), y(0), z(0) {}

Math::Vector3D::Vector3D(double _x, double _y, double _z)
    : x(_x), y(_y), z(_z) {}

double Math::Vector3D::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

Math::Vector3D Math::Vector3D::normalized() const {
    double len = length();
    if (len < 1e-8) return {0.0, 0.0, 0.0};
    return { x / len, y / len, z / len };
}

Math::Vector3D& Math::Vector3D::normalize() {
    double len = length();
    if (len >= 1e-8) {
        x /= len;
        y /= len;
        z /= len;
    }
    return *this;
}

double Math::Vector3D::dot(const Math::Vector3D& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Math::Vector3D Math::Vector3D::cross(const Math::Vector3D& o) const {
    return {
        y * o.z - z * o.y,
        z * o.x - x * o.z,
        x * o.y - y * o.x
    };
}

Math::Vector3D Math::Vector3D::operator+(const Math::Vector3D& o) const {
    return {x + o.x, y + o.y, z + o.z};
}

Math::Vector3D& Math::Vector3D::operator+=(const Math::Vector3D& o) {
    x += o.x; y += o.y; z += o.z;
    return *this;
}

Math::Vector3D Math::Vector3D::operator-(const Math::Vector3D& o) const {
    return {x - o.x, y - o.y, z - o.z};
}

Math::Vector3D& Math::Vector3D::operator-=(const Math::Vector3D& o) {
    x -= o.x; y -= o.y; z -= o.z;
    return *this;
}

Math::Vector3D Math::Vector3D::operator*(const Math::Vector3D& o) const {
    return {x * o.x, y * o.y, z * o.z};
}

Math::Vector3D& Math::Vector3D::operator*=(const Math::Vector3D& o) {
    x *= o.x; y *= o.y; z *= o.z;
    return *this;
}

Math::Vector3D Math::Vector3D::operator/(const Math::Vector3D& o) const {
    return {x / o.x, y / o.y, z / o.z};
}

Math::Vector3D& Math::Vector3D::operator/=(const Math::Vector3D& o) {
    x /= o.x; y /= o.y; z /= o.z;
    return *this;
}

Math::Vector3D Math::Vector3D::operator*(double s) const {
    return {x * s, y * s, z * s};
}

Math::Vector3D& Math::Vector3D::operator*=(double s) {
    x *= s; y *= s; z *= s;
    return *this;
}

Math::Vector3D Math::Vector3D::operator/(double s) const {
    return {x / s, y / s, z / s};
}

Math::Vector3D& Math::Vector3D::operator/=(double s) {
    x /= s; y /= s; z /= s;
    return *this;
}

Math::Vector3D& Math::Vector3D::translate(const Math::Vector3D& t) {
    x += t.x; y += t.y; z += t.z;
    return *this;
}

Math::Vector3D Math::Vector3D::rotateX(double deg) const {
    double r = deg * M_PI / 180.0;
    double c = std::cos(r), s = std::sin(r);
    return {x, y * c - z * s, y * s + z * c};
}

Math::Vector3D Math::Vector3D::rotateY(double deg) const {
    double r = deg * M_PI / 180.0;
    double c = std::cos(r), s = std::sin(r);
    return {x * c + z * s, y, -x * s + z * c};
}

Math::Vector3D Math::Vector3D::rotateZ(double deg) const {
    double r = deg * M_PI / 180.0;
    double c = std::cos(r), s = std::sin(r);
    return {x * c - y * s, x * s + y * c, z};
}