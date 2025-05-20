/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** NoiseTexture
*/

#pragma once

#include "Texture.hpp"
#include "PerlinNoise.hpp"
#include <memory>

namespace RayTracer {

    class NoiseTexture : public Texture {
    public:
        NoiseTexture(std::shared_ptr<Texture> base, float scale = 10.0f, float strength = 0.1f);

        Math::Vector3D sample(float u, float v) const override;

    private:
        std::shared_ptr<Texture> _base;
        PerlinNoise _noise;
        float _scale;
        float _strength;
    };

}
