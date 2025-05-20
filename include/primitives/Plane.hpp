/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Plane.hpp
 * @brief Declares the infinite plane primitive.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * An unbounded plane defined by a point and a normal.  Supports ray
 * intersection with a single-sided test (front-facing rays only),
 * and computes simple planar UVs for texturing.
*/

#pragma once

#include <cmath>
#include <limits>
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "rendering/BVH.hpp"

namespace RayTracer {

    /**
     * @class Plane
     * @brief Infinite, single-sided plane primitive.
     *
     * Defined by a world-space anchor point and a unit normal vector.
     * Intersections occur only when the ray approaches from the “front”
     * side (i.e.\ the direction of the normal).
    */

    class Plane : public Primitive {
    public:
        Math::Point3D  point;   ///< Any point lying on the plane. 
        Math::Vector3D normal;  ///< Unit-length normal vector pointing “out” of the plane.

        /**
         * @brief      Construct a plane with a given material.
         * @param p    A point on the plane.
         * @param n    Plane normal (will be normalized).
         * @param mat  Material to apply to the plane.
        */
        Plane(const Math::Point3D& p,
              const Math::Vector3D& n,
              const Material& mat);

        /**
         * @brief Get the plane’s bounding box.
         * @return An infinite AABB.
         *
         * @note Since the plane is unbounded, we return +/-infinite extents.
        */
        AABB bounds() const override;

        /**
         * @brief Ray–plane intersection.
         *
         * Performs a front-face test (denom = N⋅D < 0), then computes
         * t = (P₀−O)⋅N / (D⋅N).  Fills HitRecord with point, normal,
         * UV coordinates (scaled and wrapped), and material.
         *
         * @param ray   Incoming ray in world space.
         * @param tOut  [out] Ray parameter at intersection.
         * @param rec   [out] Filled with hit information.
         * @return      true if the ray hits the front face of the plane.
         * 
         * @see   Primitive, HitRecord
        */
        bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;
    };

} // namespace RayTracer
