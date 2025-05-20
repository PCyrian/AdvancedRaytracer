/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file TangleCube.hpp
 * @brief Declares the TangleCube implicit fractal surface primitive.
 * @author Thibaut Louis
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * The TangleCube is an implicit fractal surface defined by the quartic equation:  
 *     F(x, y, z) = x⁴ + y⁴ + z⁴ − α(x² + y² + z²) + β  
 * It supports ray-implicit surface intersection by solving the quartic equation  
 * using the Durand–Kerner method. The surface normal is computed analytically  
 * via the gradient of F. This primitive is typically used for complex fractal  
 * shapes in ray-traced scenes.
*/

#pragma once

#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "rendering/BVH.hpp"
#include <complex.h>

namespace RayTracer {

/**
 * @class TangleCube
 * @brief Implicit tangleCube fractal surface.
 *
 *  1. Calculate the fourth power :
 *      sum4 = x⁴ + y⁴ + z⁴
 *
 *  2. Calculate the sum of the squares and multiply it by α :
 *      sum2 = x² + y² + z²  ⇒  α · sum2
 *
 *  3. Subtract this result from sum4 and add β :
 *      F(x,y,z) = sum4 − α·sum2 + β
 *
 *  Implements a ray‐implicit intersection by solving the quartic equation.
 */
    class TangleCube : public Primitive {
    public:

        /**
         * @brief         Construct a TangleCube primitive.
         * @param center  Position of the fractal’s center in world space.
         * @param scale   Uniform scale factor for the field evaluation.
         * @param mat     Material used for shading.
        */
        TangleCube(const Math::Point3D& center, double scale, const Material& mat);

        ~TangleCube() override;
        /**
         * @brief Attempt ray–fractal intersection.
         *
         * Solves the quartic equation for F(ray(t)) = 0 via Durand–Kerner,
         * then selects the smallest positive real root.
         *
         * @param ray   Input ray in world coordinates.
         * @param tHit  Output param for the ray parameter at intersection.
         * @param rec   HitRecord to fill with point, normal, and material.
         * @return      true if intersection is found (tHit > 0), false otherwise.
         * 
         * @see   Primitive, HitRecord
        */
        bool hits(const Ray& ray, double& tHit, HitRecord& rec) const override;

        /**
         * @brief Axis‑aligned bounding box for the fractal.
         *
         * Since we only solve up to a fixed pad, we use a cube of side 2*(scale+1).
         * @return AABB in world coordinates.
        */
        AABB bounds() const override;

    private:
        Math::Point3D _center;  ///< Center location of the fractal
        double        _scale;   ///< Scale factor applied to field evaluation

        /**
         * @brief Solves a quartic equation Ax⁴ + Bx³ + Cx² + Dx + E = 0.
         *
         * Uses the Durand–Kerner method: initializes four complex guesses,
         * iterates until convergence, and returns the real parts of roots
         * with negligible imaginary components.
         *
         * @param A Coefficient of x⁴.
         * @param B Coefficient of x³.
         * @param C Coefficient of x².
         * @param D Coefficient of x.
         * @param E Constant term.
         * @return  A vector of the real roots found.
        */
       static std::vector<double> solveQuartic(double A, double B, double C, double D, double E);
    };

} // namespace RayTracer
