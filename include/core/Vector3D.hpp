/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <cmath>

namespace Math {

    struct Vector3D {
        double x, y, z;

        Vector3D();
        Vector3D(double _x, double _y, double _z);

        double length() const;
        Vector3D normalized() const;
        Vector3D& normalize();
        double dot(const Vector3D& other) const;
        Vector3D cross(const Vector3D& other) const;

        Vector3D operator+(const Vector3D& o) const;
        Vector3D& operator+=(const Vector3D& o);
        Vector3D operator-(const Vector3D& o) const;
        Vector3D& operator-=(const Vector3D& o);
        Vector3D operator*(const Vector3D& o) const;
        Vector3D& operator*=(const Vector3D& o);
        Vector3D operator/(const Vector3D& o) const;
        Vector3D& operator/=(const Vector3D& o);

        Vector3D operator*(double s) const;
        Vector3D& operator*=(double s);
        Vector3D operator/(double s) const;
        Vector3D& operator/=(double s);

        Vector3D& translate(const Vector3D& t);

        Vector3D rotateX(double deg) const;
        Vector3D rotateY(double deg) const;
        Vector3D rotateZ(double deg) const;
    };

}
