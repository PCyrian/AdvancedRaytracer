/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Sphere.hpp"

namespace RayTracer {

    AABB Sphere::bounds() const
    {
        Math::Vector3D r{radius, radius, radius};
        return AABB{center - r, center + r};
    }

    Sphere::Sphere(const Math::Point3D& c,
            double r,
            const Math::Vector3D& col)
            : Primitive(Material{"legacy", col, 0.0f, 0.0f,
                                    Math::Vector3D{0.0, 0.0, 0.0}, 0.0f, 1.0f}),
                center(c), radius(r) {}
    
    Sphere::Sphere(const Math::Point3D& c,
            double r,
            const Material& mat)
            : Primitive(mat),
                center(c), radius(r) {}

    bool Sphere::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        Math::Vector3D oc = ray.origin - center;
        double a = ray.direction.dot(ray.direction);
        double b = 2.0 * oc.dot(ray.direction);
        double c = oc.dot(oc) - radius * radius;
        double disc = b*b - 4*a*c;
        if (disc < 0) return false;
        double sq = std::sqrt(disc);
        double root = (-b - sq) / (2*a);
        if (root < 1e-8) {
            root = (-b + sq) / (2*a);
            if (root < 1e-8) return false;
        }
        tOut = root;
        rec.t = root;
        rec.point = ray.origin + ray.direction * root;
        rec.normal = (rec.point - center) / radius;
        rec.material = material;

        Math::Vector3D p = (rec.point - center) / radius;
        double theta = std::atan2(p.z, p.x);
        double phi   = std::acos(p.y);
        float u = static_cast<float>((theta + M_PI) / (2.0 * M_PI));
        float v = static_cast<float>(phi / M_PI);
        rec.uv = { u, v };

        return true;
    }
};// namespace RayTracer
