/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once
#include "Point3D.hpp"
#include "Vector3D.hpp"

namespace RayTracer {
    struct Ray {
        Math::Point3D origin;
        Math::Vector3D direction;

        Ray() = default;
        Ray(const Math::Point3D& o, const Math::Vector3D& d)
            : origin(o), direction(d) {}
    };
}
