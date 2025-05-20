/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Triangle.hpp
 * @brief Declares the Triangle primitive and its ray–triangle intersection logic.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * The Triangle primitive represents a flat surface defined by three vertices (v0, v1, v2).  
 * It implements ray–triangle intersection using the Möller–Trumbore algorithm,  
 * which efficiently computes the intersection point, surface normal, and optional hit record.  
 * 
 * Supports both back-face culling and no-cull intersection tests.
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
 * @class Triangle
 * @brief Triangle primitive for ray tracing.
 *
 * Represents a triangle defined by three vertices.  
 * Implements fast intersection tests using the Möller–Trumbore algorithm.  
 * Can be used in mesh representations or directly in scene geometry.
*/
class Triangle : public Primitive {
public:
    Math::Point3D v0;  ///< First vertex of the triangle
    Math::Point3D v1;  ///< Second vertex of the triangle
    Math::Point3D v2;  ///< Third vertex of the triangle

    /**
     * @brief Compute the axis-aligned bounding box enclosing the triangle.
     * @return AABB tightly enclosing the triangle.
    */
    AABB bounds() const override;

    /**
     * @brief Construct a legacy-colored triangle.
     * @param _v0 First vertex.
     * @param _v1 Second vertex.
     * @param _v2 Third vertex.
     * @param col Diffuse color for legacy mode.
    */
    Triangle(const Math::Point3D& _v0,
             const Math::Point3D& _v1,
             const Math::Point3D& _v2,
             const Math::Vector3D& col = {1.0,1.0,1.0});

    /**
     * @brief Construct a triangle with a custom material.
     * @param _v0 First vertex.
     * @param _v1 Second vertex.
     * @param _v2 Third vertex.
     * @param mat Material applied to the triangle surface.
    */
    Triangle(const Math::Point3D& _v0,
             const Math::Point3D& _v1,
             const Math::Point3D& _v2,
             const Material& mat);

    /**
     * @brief Ray–triangle intersection test without hit record.
     * 
     * Only computes if a hit occurs and outputs the distance `tOut`.
     *
     * @param ray   Incoming ray in world coordinates.
     * @param tOut  (out) Distance along the ray to the hit point.
     * @return      true if the ray intersects the triangle.
    */
    bool hits(const Ray& ray, double& tOut) const;

    /**
     * @brief Ray–triangle intersection test with hit record.
     *
     * Uses the Möller–Trumbore algorithm with back-face culling.  
     * Populates the hit record with hit point, normal, and material.
     *
     * @param ray   Incoming ray in world coordinates.
     * @param tOut  (out) Distance along the ray to the hit point.
     * @param rec   (out) HitRecord with intersection data.
     * @return      true if the ray intersects the triangle.
     * 
     * @see   Primitive, HitRecord
    */
    bool hits(const Ray& ray, double& tOut, HitRecord& rec) const override;

    /**
     * @brief Ray–triangle intersection without back-face culling.
     *
     * Useful for double-sided surfaces or when rendering transparent meshes.
     *
     * @param ray   Incoming ray in world coordinates.
     * @param tOut  (out) Distance along the ray to the hit point.
     * @param rec   (out) HitRecord with intersection data.
     * @return      true if the ray intersects the triangle.
    */
    bool hitsNoCull(const Ray& ray, double& tOut, HitRecord& rec) const;
};

} // namespace RayTracer
