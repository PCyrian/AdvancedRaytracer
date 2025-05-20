/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/Mesh.hpp"

RayTracer::Mesh::Mesh() = default;

void RayTracer::Mesh::addTriangle(const Triangle& triangle) {
    triangles.push_back(triangle);
}

bool RayTracer::Mesh::hits(const Ray& ray, double& tOut, HitRecord& rec) const {
    bool hitAnything = false;
    double closestSoFar = tOut;
    HitRecord tempRec;

    for (const auto& triangle : triangles) {
        double tTmp;
        HitRecord recTmp;
        if (triangle.hits(ray, tTmp, recTmp) && tTmp < closestSoFar) {
            hitAnything = true;
            closestSoFar = tTmp;
            tempRec = recTmp;
        }
    }

    if (hitAnything) {
        rec = tempRec;
        tOut = closestSoFar;
    }
    return hitAnything;
}

void RayTracer::Mesh::transform(const Math::Vector3D& translation, double scale) {
    for (auto& tri : triangles) {
        tri.v0 = (tri.v0 * scale) + translation;
        tri.v1 = (tri.v1 * scale) + translation;
        tri.v2 = (tri.v2 * scale) + translation;
    }
}