/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Triangle.hpp"

namespace RayTracer {

    AABB Triangle::bounds() const
    {
        double minX = std::min({ v0.x, v1.x, v2.x });
        double minY = std::min({ v0.y, v1.y, v2.y });
        double minZ = std::min({ v0.z, v1.z, v2.z });
        double maxX = std::max({ v0.x, v1.x, v2.x });
        double maxY = std::max({ v0.y, v1.y, v2.y });
        double maxZ = std::max({ v0.z, v1.z, v2.z });
        return AABB{ {minX, minY, minZ}, {maxX, maxY, maxZ} };
    }

    Triangle::Triangle(const Math::Point3D& _v0,
        const Math::Point3D& _v1,
        const Math::Point3D& _v2,
        const Math::Vector3D& col)
        : Primitive(Material{"legacy", col, 0.0f, 0.0f, Math::Vector3D{0.0,0.0,0.0}, 0.0f, 1.0f}),
        v0(_v0), v1(_v1), v2(_v2)
        {}

    Triangle::Triangle(const Math::Point3D& _v0,
                const Math::Point3D& _v1,
                const Math::Point3D& _v2,
                const Material& mat)
        : Primitive(mat),
        v0(_v0), v1(_v1), v2(_v2)
    {}

    bool Triangle::hits(const Ray& ray, double& tOut) const
    {
        HitRecord rec;
        return hits(ray, tOut, rec);
    }

    /* Updated Triangle::hits in Triangle.cpp */

    bool Triangle::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        // Möller–Trumbore intersection
        auto edge1 = v1 - v0;
        auto edge2 = v2 - v0;
        auto pvec  = ray.direction.cross(edge2);
        double det = edge1.dot(pvec);
        if (det > -EPS && det < EPS) return false;
        double invDet = 1.0 / det;

        auto tvec = ray.origin - v0;
        double u = tvec.dot(pvec) * invDet;
        if (u < 0.0 || u > 1.0) return false;

        auto qvec = tvec.cross(edge1);
        double v = ray.direction.dot(qvec) * invDet;
        if (v < 0.0 || u + v > 1.0) return false;

        double t = edge2.dot(qvec) * invDet;
        if (t <= EPS) return false;

        // Fill hit record
        tOut = rec.t = t;
        rec.point  = ray.origin + ray.direction * t;
        rec.normal = edge1.cross(edge2).normalized();
        rec.material = material;
        rec.color    = material.albedo;

        // Auto-detect projection plane based on triangle normal
        auto n = rec.normal;
        auto absN = Math::Vector3D(std::fabs(n.x), std::fabs(n.y), std::fabs(n.z));
        double scale = 50.0; // world units per texture tile
        float uTex, vTex;
        if (absN.z >= absN.x && absN.z >= absN.y) {
            // most orthogonal to Z axis: project X-Y (wall orientation)
            uTex = static_cast<float>(rec.point.x / scale);
            vTex = static_cast<float>(rec.point.y / scale);
        } else if (absN.y >= absN.x) {
            // most orthogonal to Y axis: project X-Z (floor/ceiling)
            uTex = static_cast<float>(rec.point.x / scale);
            vTex = static_cast<float>(rec.point.z / scale);
        } else {
            // most orthogonal to X axis: project Y-Z
            uTex = static_cast<float>(rec.point.y / scale);
            vTex = static_cast<float>(rec.point.z / scale);
        }
        rec.uv = { uTex, vTex };

        return true;
    }



    bool Triangle::hitsNoCull(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        auto edge1 = v1 - v0;
        auto edge2 = v2 - v0;
        auto faceN = edge1.cross(edge2);

        auto h = ray.direction.cross(edge2);
        auto a = edge1.dot(h);
        if (std::abs(a) < EPS) return false;

        auto f = 1.0 / a;
        auto s = ray.origin - v0;
        auto u = f * s.dot(h);
        if (u < 0.0 || u > 1.0) return false;

        auto q = s.cross(edge1);
        auto v = f * ray.direction.dot(q);
        if (v < 0.0 || u + v > 1.0) return false;

        auto tmp = f * edge2.dot(q);
        if (tmp <= EPS) return false;

        tOut = tmp;
        rec.t = tmp;
        rec.point = ray.origin + ray.direction * tmp;
        rec.normal = faceN / faceN.length();
        rec.material = material;
        rec.color = material.albedo;
        return true;
    }
}; // namespace RayTracer
    