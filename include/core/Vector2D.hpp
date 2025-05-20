/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

namespace Math {
    struct Vector2D {
        float x = 0.0f;
        float y = 0.0f;

        Vector2D() = default;
        Vector2D(float _x, float _y) : x(_x), y(_y) {}
    };
}