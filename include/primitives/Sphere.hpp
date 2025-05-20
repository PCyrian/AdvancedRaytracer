/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Sphere.hpp
 * @brief Declares the Sphere primitive for ray tracing.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * A sphere primitive defined by a center point and a radius.
 * Supports ray-sphere intersection tests and computes UV coordinates
 * based on spherical mapping.
*/

#pragma once

#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "rendering/BVH.hpp"

#include <cmath>

namespace RayTracer {

    /**
     * @class Sphere
     * @brief Sphere primitive for ray tracing.
     *
     * A sphere is defined by its center position and radius.  
     * Ray-sphere intersection solves the quadratic analytically and 
     * computes UV texture coordinates based on spherical projection.
    */
    class Sphere : public Primitive {
    public:
        Math::Point3D center;   ///< Center of the sphere in world space
        double radius;          ///< Radius of the sphere

        /**
         * @brief Compute the axis-aligned bounding box for the sphere.
         * @return AABB that tightly encloses the sphere.
        */
        AABB bounds() const override;

        /**
         * @brief      Construct a legacy-colored sphere.
         * @param c    Center of the sphere in world space.
         * @param r    Radius of the sphere.
         * @param col  Diffuse color for legacy mode.
        */
        Sphere(const Math::Point3D& c, double r, const Math::Vector3D& col);
        
        /**
         * @brief Construct a sphere with a custom material.
         * @param c    Center of the sphere in world space.
         * @param r    Radius of the sphere.
         * @param mat  Material for the sphere surface.
        */
        Sphere(const Math::Point3D& c, double r, const Material& mat);

        /**
         * @brief Ray-sphere intersection test.
         *
         * Analytically solves the quadratic equation for intersection.  
         * Computes the surface normal and UV coordinates using spherical mapping.
         *
         * @param ray   The incoming ray in world coordinates.
         * @param tOut  (out) Ray parameter of the hit point.
         * @param rec   (out) HitRecord filled with point, normal, material, uv.
         * @return      true if the ray intersects the sphere.
         * 
         * @see   Primitive, HitRecord
        */
        bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;
    };

} // namespace RayTracer
