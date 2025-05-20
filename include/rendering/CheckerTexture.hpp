/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** CheckerTexture
*/

#pragma once

#include "Texture.hpp"
#include <cmath>

namespace RayTracer {

    class CheckerTexture : public Texture {
        Math::Vector3D _c1, _c2;
        float _scale;

    public:
        CheckerTexture(const Math::Vector3D& c1,
                       const Math::Vector3D& c2,
                       float scale);

        Math::Vector3D sample(float u, float v) const override;
    };

}
