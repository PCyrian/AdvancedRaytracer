/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Cone.hpp
 * @brief Declares the finite (capped) right‐circular cone primitive.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * A cone defined by its apex point, a unit‐length axis direction,
 * an apex half‐angle, and a finite height.  Supports intersection
 * against both the conical side and the circular base cap, and
 * computes UV coordinates for simple texturing.
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
     * @class Cone
     * @brief  Finite (capped) right‐circular cone primitive.
     */
    class Cone : public Primitive {
    public:
        Math::Point3D apex;    ///< Cone apex point
        Math::Vector3D axis;   ///< Unit‐length axis (apex → base)
        double         angle;  ///< Apex half‐angle (radians)
        double         height; ///< Distance from apex down to base

        /** 
         * @brief Construct a legacy‐colored cone.
         * @param a      Apex location.
         * @param ax     Axis direction (will be normalized).
         * @param angDeg Half‐angle at apex, in degrees.
         * @param h      Height from apex to base.
         * @param col    Flat diffuse color.
        */
        Cone(const Math::Point3D& a,
             const Math::Vector3D& ax,
             double angDeg,
             double h,
             const Math::Vector3D& col);

        /** 
         * @brief Construct a cone with a custom material.
         * @param a      Apex location.
         * @param ax     Axis direction (will be normalized).
         * @param angDeg Half‐angle at apex, in degrees.
         * @param h      Height from apex to base.
         * @param mat    Material used for both side and cap.
        */
        Cone(const Math::Point3D& a,
             const Math::Vector3D& ax,
             double angDeg,
             double h,
             const Material& mat);

        /** 
         * @brief Compute axis‐aligned bounding box of this cone.
         * @return AABB enclosing both the side surface and the base cap.
        */
        AABB bounds() const override;

        /**
         * @brief Ray–cone intersection: side + base cap.
         * @param ray   The incident ray.
         * @param tOut  [out] Ray‐parameter at intersection.
         * @param rec   [out] Hit record (point, normal, uv, material).
         * @return      True if the ray hits the cone.
        */
        bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;
    };

} // namespace RayTracer