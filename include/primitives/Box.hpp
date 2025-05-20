/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Box.hpp
 * @brief Declares the axis-aligned Box primitive and its ray‐intersection logic.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * This primitive represents a rectangular box defined by two opposite
 * corners (`min`, `max`). It implements the slab‐method for ray‐box
 * intersection and computes simple UV coordinates for texturing.
*/

#pragma once

#include <algorithm>
#include <cmath>

#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "rendering/BVH.hpp"

namespace RayTracer {

/**
 * @class Box
 * @brief Axis-aligned box primitive with ray intersection.
 */
class Box : public Primitive {
public:
    Math::Point3D min, max;

    Box(const Math::Point3D& mn,
        const Math::Point3D& mx,
        const Math::Vector3D& col);

    Box(const Math::Point3D& mn,
        const Math::Point3D& mx,
        const Material& mat);

    AABB bounds() const override;

    /**
     * @brief Ray–box intersection using the slab method.
     * @param ray   incoming ray
     * @param tOut  (out) hit distance
     * @param rec   (out) hit record
     * @return      true if hit
     */
    bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;
};

} // namespace RayTracer