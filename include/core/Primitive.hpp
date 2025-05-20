/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include "core/Ray.hpp"
#include "core/Point3D.hpp"
#include "core/Vector2D.hpp"
#include "core/Vector3D.hpp"
#include "rendering/Material.hpp"

namespace RayTracer {
    struct AABB;

    struct HitRecord {
        double t;
        Math::Point3D point;
        Math::Vector3D normal;
        Material material;
        Math::Vector3D color;
        Math::Vector2D uv;
    };

    class Primitive {
    protected:
        Material material;
    public:
        Primitive(const Material& mat) : material(mat) {}
        virtual ~Primitive() = default;

        virtual bool hits(const Ray& ray, double& t, HitRecord& rec) const = 0;
        virtual AABB bounds()     const = 0;

        const Material& getMaterial() const;
    };

} // namespace RayTracer
