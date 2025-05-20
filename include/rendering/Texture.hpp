/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Texture
*/

#pragma once

#include "core/Vector3D.hpp"

namespace RayTracer {
    struct Texture {
        virtual ~Texture() = default;

        virtual Math::Vector3D sample(float u, float v) const = 0;
    };
}
