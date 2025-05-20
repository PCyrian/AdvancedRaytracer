/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Box.hpp"

namespace RayTracer {

    Box::Box(const Math::Point3D& mn, const Math::Point3D& mx, const Math::Vector3D& col)
    : Primitive(Material{"legacy", col, 0.0f, 0.0f, {0,0,0}, 0.0f, 1.0f}), min(mn), max(mx) {}

    Box::Box(const Math::Point3D& mn, const Math::Point3D& mx, const Material& mat)
    : Primitive(mat), min(mn), max(mx) {}

    AABB Box::bounds() const 
    {
        return AABB{ min, max };
    }

    bool Box::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        double tmin = (min.x - ray.origin.x) / ray.direction.x;
        double tmax = (max.x - ray.origin.x) / ray.direction.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        double tymin = (min.y - ray.origin.y) / ray.direction.y;
        double tymax = (max.y - ray.origin.y) / ray.direction.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if (tmin > tymax || tymin > tmax) return false;
        tmin = std::max(tmin, tymin);
        tmax = std::min(tmax, tymax);

        double tzmin = (min.z - ray.origin.z) / ray.direction.z;
        double tzmax = (max.z - ray.origin.z) / ray.direction.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if (tmin > tzmax || tzmin > tmax) return false;
        tmin = std::max(tmin, tzmin);
        tmax = std::min(tmax, tzmax);

        double t = (tmin >= EPS ? tmin : (tmax >= EPS ? tmax : -1));
        if (t < 0) return false;

        tOut = t;
        rec.t = t;
        rec.point = ray.origin + ray.direction * t;

        const double e = 1e-6;
        auto &p = rec.point;
        if (std::abs(p.x - min.x) < e) rec.normal = { -1,  0,  0 };
        else if (std::abs(p.x - max.x) < e) rec.normal = {  1,  0,  0 };
        else if (std::abs(p.y - min.y) < e) rec.normal = {  0, -1,  0 };
        else if (std::abs(p.y - max.y) < e) rec.normal = {  0,  1,  0 };
        else if (std::abs(p.z - min.z) < e) rec.normal = {  0,  0, -1 };
        else  rec.normal = {  0,  0,  1 };

        double tileSize = 80.0; // mm per brick
        double u, v;
        if (std::abs(rec.normal.x) > 0.5) {         // X-faces
            u = (rec.point.z - min.z) / tileSize;
            v = (rec.point.y - min.y) / tileSize;
        } else if (std::abs(rec.normal.y) > 0.5) {  // Y-faces
            u = (rec.point.x - min.x) / tileSize;
            v = (rec.point.z - min.z) / tileSize;
        } else {                                   // Z-faces
            u = (rec.point.x - min.x) / tileSize;
            v = (rec.point.y - min.y) / tileSize;
        }
        rec.uv = { static_cast<float>(u), static_cast<float>(v) };

        rec.material = material;
        rec.color = material.albedo;
        return true;
    }

} // namespace RayTracer
