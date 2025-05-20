/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** PerlinNoise
*/

#pragma once

#include <vector>
#include <numeric>
#include <random>
#include <algorithm>

namespace RayTracer {

    class PerlinNoise {
    public:
        PerlinNoise();

        float noise(float x, float y) const;

    private:
        std::vector<int> p;

        float fade(float t) const;
        float lerp(float t, float a, float b) const;
        float grad(int hash, float x, float y) const;
    };

}
