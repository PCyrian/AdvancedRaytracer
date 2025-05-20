/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Cylinder.hpp
 * @brief Declares the finite right‐circular Cylinder primitive.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * A capped cylinder defined by a base center point, an axis direction,
 * a radius and a finite height.  Ray‐intersection handles both the
 * curved side and the two circular end caps, and computes UV coordinates.
 */

#pragma once

#include <cmath>
#include <limits>
#include <algorithm>

#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "rendering/BVH.hpp"

namespace RayTracer {

    /**
     * @class Cylinder
     * @brief  Finite right-circular cylinder primitive.
     *
     * A cylinder is defined by a base point, an axis direction,
     * a radius, and a height.  Ray–cylinder intersection tests both
     * the curved side and the two end caps, and computes UV coordinates.
    */
    class Cylinder : public Primitive {
    public:
        Math::Point3D  base;    ///< Center of the bottom cap.
        Math::Vector3D axis;    ///< Unit axis direction from base toward top.
        double          radius; ///< Radius of the circular section.
        double          height; ///< Height of the cylinder along the axis.

        /**
         * @brief     Construct a legacy-colored cylinder.
         * @param b   Base center in world space.
         * @param ax  Axis direction (will be normalized).
         * @param r   Radius of the cylinder.
         * @param h   Height along the axis.
         * @param col Diffuse color for legacy mode.
        */
        Cylinder(const Math::Point3D& b, const Math::Vector3D& ax, double r, double h, const Math::Vector3D& col);

        /**
         * @brief     Construct a cylinder with a custom material.
         * @param b   Base center in world space.
         * @param ax  Axis direction (will be normalized).
         * @param r   Radius of the cylinder.
         * @param h   Height along the axis.
         * @param mat Material for both side and caps.
        */
        Cylinder(const Math::Point3D& b, const Math::Vector3D& ax, double r, double h, const Material& mat);

        /**
         * @brief Compute the axis-aligned bounding box.
         * @return AABB that encloses the entire cylinder (side + caps).
        */
        AABB bounds() const override;

        /**
         * @brief  Ray–cylinder intersection test.
         *
         * Solves the quadratic for the infinite cylinder sides, clamps to [0,height],
         * then separately tests the two circular end caps.  Chooses the nearest hit
         * and computes the surface normal and UV coordinates.
         *
         * @param ray   The incoming ray in world coordinates.
         * @param tOut  [out] Ray parameter of the hit point.
         * @param rec   [out] HitRecord filled with point, normal, material, uv.
         * @return      true if the ray intersects the cylinder.
         * @see         Primitive, HitRecord
        */
        bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;
    };

} // namespace RayTracer
