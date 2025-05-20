/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once
#include "Vector3D.hpp"
#include "cmath"

namespace Math {
    struct Point3D {
        double x = 0, y = 0, z = 0;
        Point3D() = default;
        Point3D(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

        friend Point3D operator+(const Point3D& p, const Vector3D& v) {
            return {p.x + v.x, p.y + v.y, p.z + v.z};
        }
        friend Point3D operator-(const Point3D& p, const Vector3D& v) {
            return {p.x - v.x, p.y - v.y, p.z - v.z};
        }
        friend Vector3D operator-(const Point3D& a, const Point3D& b) {
            return {a.x - b.x, a.y - b.y, a.z - b.z};
        }
        friend Point3D operator*(const Point3D& p, double s) {
            return {p.x * s, p.y * s, p.z * s};
        }
    };
}
