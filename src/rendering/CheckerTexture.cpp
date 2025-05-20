/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/CheckerTexture.hpp"

#include <cmath>

RayTracer::CheckerTexture::CheckerTexture(const Math::Vector3D& c1,
    const Math::Vector3D& c2,
    float scale) : _c1(c1), _c2(c2), _scale(scale) {

    }

Math::Vector3D RayTracer::CheckerTexture::sample(float u, float v) const {
    float iu = std::floor(u * _scale);
    float iv = std::floor(v * _scale);
    if (std::fmod(iu + iv, 2.0f) < 1.0f)
        return _c1;
    else
        return _c2;
}
