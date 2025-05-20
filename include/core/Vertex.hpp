/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <limits>
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Vector2D.hpp"
#include "core/Primitive.hpp"

namespace RayTracer {
    enum class VertexType {
        Surface,
        Light,
        Camera
    };

    struct Vertex {
        VertexType type = VertexType::Surface;
        HitRecord rec;
        Math::Vector3D wi {0.0, 0.0, 0.0};
        Math::Vector3D beta {1.0, 1.0, 1.0}; // throughput
        double pdfFwd = 0.0; // forward PDF
        double pdfRev = 0.0; // reverse PDF
        bool delta = false; // delta-scatter?
        bool is_point_light_source = false; // true if this vertex is a point light

        Vertex() = default;
    };

} // namespace RayTracer
