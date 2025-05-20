/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Menger.hpp
 * @brief Declares the Menger Sponge fractal primitive.
 * @author Noé Carabin
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * This class recursively generates a Menger Sponge at a given center,
 * size, and subdivision depth by spawning smaller Box primitives.
 * Ray‐intersection tests sweep all children and pick the nearest hit.
 */


#pragma once

#include <vector>
#include <memory>
#include <cmath>
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "Box.hpp"

namespace RayTracer {

    /**
     * @class Menger
     * @brief Fractal “Menger Sponge” built from axis-aligned boxes.
     *
     * A Menger Sponge centered at a given point, with a specified
     * outer size and recursion depth.  At each level, the current
     * cube is subdivided into 27 smaller cubes, removing those in
     * the “hole” positions.
    */
    class Menger : public Primitive {
    public:
        /**
         * @brief         Constructs a Menger Sponge.
         * @param center  World-space center of the sponge.
         * @param size    Edge length of the initial cube.
         * @param depth   Number of recursive subdivisions.
         * @param mat     Material applied to all child boxes.
        */
        Menger(const Math::Point3D& center,
               double size,
               int depth,
               const Material& mat);

        /**
         * @brief       Ray–sponge intersection: tests every child box.
         * @param ray   Incoming ray in world space.
         * @param tHit  [out] Parameter *t* of the closest hit.
         * @param rec   [out] Filled with hit information of the closest child.
         * @return      True if any child box is hit.
        */
        bool hits(const Ray& ray, double& tHit, HitRecord& rec) const override;

        /**
         * @brief Compute the axis-aligned bounding box of the sponge.
         * @return AABB enclosing all child boxes.
        */
        AABB bounds() const override;

    private:
        std::vector<std::shared_ptr<Primitive>> _children; ///< All generated child boxes

        /** @brief Should the sub-cube at (x,y,z) be removed? */
        bool isRemoved(int x, int y, int z) const;

        /**
         * @brief         Recursively generate sub-cubes.
         * @param center  Center of the current cube.
         * @param size    Edge length of the current cube.
         * @param depth   Remaining subdivision levels.
        */
        void generate(const Math::Point3D& center, double size, int depth);
    };

} // namespace RayTracer
