/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Mesh
*/

#pragma once

#include <vector>

#include "primitives/Triangle.hpp"
#include "core/Ray.hpp"

namespace RayTracer {
    class Mesh {
    public:
        std::vector<Triangle> triangles;

        Mesh();

        void addTriangle(const Triangle& triangle);
        bool hits(const Ray& ray, double& tOut, HitRecord& rec) const;
        void transform(const Math::Vector3D& translation, double scale);
    };
}
