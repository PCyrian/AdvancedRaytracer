/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Light
*/

#pragma once

#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"

namespace RayTracer {

    enum class LightType { Ambient, Directional, Point };

    struct Light {
        LightType      type;
        Math::Vector3D direction;
        Math::Point3D  position;
        double         intensity;

        static Light ambient(double i);
        static Light directional(const Math::Vector3D& dir, double i);
        static Light point(const Math::Point3D& pos, double i);
    };

}
