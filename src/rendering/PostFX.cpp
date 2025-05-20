/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/PostFX.hpp"

#include <algorithm>

float PostFX::clamp01(float v) {
    return v < 0.f ? 0.f : (v > 1.f ? 1.f : v);
}

Math::Vector3D PostFX::adjustContrast(const Math::Vector3D& c, float contrast) {
    return {
        clamp01((c.x - 0.5f) * contrast + 0.5f),
        clamp01((c.y - 0.5f) * contrast + 0.5f),
        clamp01((c.z - 0.5f) * contrast + 0.5f)
    };
}

Math::Vector3D PostFX::adjustSaturation(const Math::Vector3D& c, float satF) {
    float r = clamp01((float)c.x), g = clamp01((float)c.y), b = clamp01((float)c.z);
    float maxv = std::max({r, g, b});
    float minv = std::min({r, g, b});
    float l = (maxv + minv) * 0.5f;
    float s = (maxv == minv) ? 0.0f :
              (l < 0.5f ? (maxv - minv) / (maxv + minv)
                        : (maxv - minv) / (2.0f - maxv - minv));
    s = clamp01(s * satF);

    Math::Vector3D gray{l, l, l};
    return gray * (1.0f - s) + c * s;
}

Math::Vector3D PostFX::toneMap(const Math::Vector3D& c) {
    return {
        c.x / (1.0 + c.x),
        c.y / (1.0 + c.y),
        c.z / (1.0 + c.z)
    };
}

float PostFX::vignetteFactor(int i, int j, int w, int h, float strength) {
    float x = 2.0f * i / (w - 1) - 1.0f;
    float y = 2.0f * j / (h - 1) - 1.0f;
    float d = std::sqrt(x*x + y*y);
    return clamp01(1.0f - d * strength);
}

void PostFX::postProcess(std::vector<Math::Vector3D>& fb,
                         int w, int h,
                         float contrast,
                         float saturation,
                         float vignetteStrength)
{
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            int idx = j * w + i;
            auto c = fb[idx];
            c = adjustContrast(c, contrast);
            c = adjustSaturation(c, saturation);
            c = toneMap(c);
            float v = vignetteFactor(i, j, w, h, vignetteStrength);
            fb[idx] = c * v;
        }
    }
}

