/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/NoiseTexture.hpp"

using namespace RayTracer;

NoiseTexture::NoiseTexture(std::shared_ptr<Texture> base,
                           float scale,
                           float strength)
  : _base(std::move(base)), _scale(scale), _strength(strength) {}

Math::Vector3D NoiseTexture::sample(float u, float v) const {
    Math::Vector3D baseColor = _base->sample(u, v);
    float n = _noise.noise(u * _scale, v * _scale);
    float factor = 1.0f + (n - 0.5f) * 2.0f * _strength;
    return baseColor * factor;
}
