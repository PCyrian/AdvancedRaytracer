/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Plane.hpp"

namespace RayTracer {

    Plane::Plane(const Math::Point3D& p,
            const Math::Vector3D& n,
            const Material& mat)
        : Primitive(mat),
        point(p),
        normal(n / n.length())
    {}

    AABB Plane::bounds() const
    {
        double inf = std::numeric_limits<double>::infinity();
        return AABB{ {-inf, -inf, -inf}, { inf,  inf,  inf} };
    }

    bool Plane::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        double denom = normal.dot(ray.direction);
        if (denom > -EPS) return false;

        double t = (point - ray.origin).dot(normal) / denom;
        if (t <= EPS) return false;

        rec.t = t;
        rec.point = ray.origin + ray.direction * t;
        rec.normal = normal;
        rec.material = this->material;

        Math::Vector3D tangent = std::abs(normal.x) > 0.9
                                    ? Math::Vector3D{0, 0, 1}
                                    : Math::Vector3D{1, 0, 0};
        Math::Vector3D tmp = normal.cross(tangent);
        double len = tmp.length();
        Math::Vector3D uAxis = tmp / len;
        Math::Vector3D vTmp = normal.cross(uAxis);
        double len2 = vTmp.length();
        Math::Vector3D vAxis = vTmp / len2;

        Math::Vector3D local = rec.point - point;
        float scale = 0.25f;
        float u = std::fmod(local.dot(uAxis) * scale, 1.0f);
        float v = std::fmod(local.dot(vAxis) * scale, 1.0f);
        if (u < 0) u += 1.0f;
        if (v < 0) v += 1.0f;
        rec.uv = {u, v};

        tOut = t;
        return true;
    }
}; // namespace RayTracer
