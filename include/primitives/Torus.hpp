/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Torus.hpp
 * @brief Declares the Torus primitive and its ray–torus intersection logic.
 * @author Thibaut Louis
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * The Torus primitive represents a 3D toroidal surface defined by a major radius
 * (distance from the center to the tube center)  
 * and a minor radius (radius of the tube). It implements ray–torus intersection by
 * numerically solving the implicit surface equation:  
 * 
 *     F(x, y, z) = (x² + y² + z² + R² − r²)² − 4·R²·(x² + z²) = 0
 * 
 * The intersection is computed via numerical root finding using interval
 * scanning and bisection refinement.  
 * UV coordinates are assigned using angular projections, and surface normals are
 * calculated analytically from the implicit function’s gradient.
*/

#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <algorithm>
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include <iostream>
#include <complex>
#include "rendering/BVH.hpp"

namespace RayTracer {

/**
 * @class Torus
 * @brief 3D primitives representating a Torus.
 *
 * A Torus with a big radius (majorRadius) and a little one (minorRadius)
 * Center with the variable center, the intersexction is numerical calculate
*/

    class Torus : public Primitive {
    public:

        /**
         * @brief               Constructor of the Torus class.
         * @param center        Center the Torus in the map.
         * @param majorRadius   Big radius (distance center -> tube).
         * @param minorRadius   Little radius (tube radius).
         * @param material      Material used for display.
        */
        Torus(const Math::Point3D& c,
              double majorRadius,
              double minorRadius,
              const Material& mat);

        /**
         * @brief       Trys intersection between radius/torus.
         * @param ray   The radius to test.
         * @param tHit  (out) distance t of the impact point.
         * @param rec   (out) hit record fill if collision.
         * @return      true if an intersection if found (t > 0).
        */
        virtual bool hits(const Ray& ray, double& tHit, HitRecord& rec) const override;

        AABB bounds() const override;

        /**
         * @brief  Evaluates the torus’s implicit surface function F(P(t)).
         *
         *  F(P) = (x² + y² + z² + R² − r²)² − 4R²·(x² + z²),
         *  where P = O + tD.
         *
         * @param O        Ray origin in torus-local coordinates.
         * @param D        Normalized ray direction.
         * @param t        Ray parameter (distance along D).
         * @param majorR   Torus major radius (center -> tube center).
         * @param minorR   Torus minor radius (tube radius).
         * @return         The value F(P(t)); zero when P lies on the torus surface.
        */

        static double torusField(const Math::Vector3D& O, const Math::Vector3D& D, double t,
            double majorR, double minorR);

        /**
         * @brief  Attempts a ray–torus intersection by numerically solving F(P(t))=0.
         *
         *  Scans the interval [tMinSearch, tMaxSearch] in fixed steps,
         *  looks for a sign change in torusField(), and then refines the root by bisection.
         *
         * @param O            Ray origin in torus-local space.
         * @param D            Normalized ray direction.
         * @param R            Torus major radius (center-to-tube-center).
         * @param r            Torus minor radius (tube radius).
         * @param tMinSearch   Lower bound of t to scan.
         * @param tMaxSearch   Upper bound of t to scan.
         * @param[out] tHit    On success, the t-value of the intersection.
         * @return             true if an intersection was found and tHit set.
         * 
         * @see   Primitive, HitRecord
        */

        static bool intersectTorus(const Math::Vector3D& O, const Math::Vector3D& D, double R, double r,
            double tMinSearch, double tMaxSearch, double& tHit);

        Math::Point3D center;   ///< Torus center
        double R;               ///< Big radius
        double r;               ///< Little radius
    
    private:

        /**
         * @brief Epsilon tolerance for floating-point comparisons (≈0).
         *
         * Used whenever we need to test if a computed value is essentially zero
         * (e.g. cubic/quartic discriminants, near-zero denominators, etc.).
        */
        static constexpr double EPS = 1e-9;

        /**
         * @brief The constant π (pi).
         *
         * Used for any trigonometric or analytic solver that needs π.
        */
        static constexpr double PI = 3.14159265358979323846;
        
        /**
         * @brief Solves a cubic equation Ax³ + Bx² + Cx + D = 0 for its principal real root.
         *
         * - If A ≈ 0, falls back to quadratic solve.
         * - Otherwise uses depressed-cubic Cardano’s formula.
         *
         * @param A Coefficient of x³.
         * @param B Coefficient of x².
         * @param C Coefficient of x¹.
         * @param D Constant term.
         * @return  The primary real solution.
        */

        static double solveCubicReal(double A, double B, double C, double D);
    
        /**
         * @brief Solves a quadratic equation ax² + bx + c = 0.
         *
         * @param a Quadratic coefficient.
         * @param b Linear coefficient.
         * @param c Constant term.
         * @return  A vector of real roots (0, 1 or 2 values).
        */

        static std::vector<double> solveQuadratic(double a, double b, double c);
    
        /**
         * @brief Computes all real roots of a cubic ax³ + bx² + cx + d = 0.
         *
         * Uses trigonometric (Casus irreducibilis) formula when discriminant < 0.
         *
         * @param a Coefficient of x³.
         * @param b Coefficient of x².
         * @param c Coefficient of x.
         * @param d Constant term.
         * @return  A vector of the real roots (either 1 or 3 values).
        */

        static std::vector<double> solveCubicAllReal(double a, double b, double c, double d);
        
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
         * @return A vector of the real roots found.
        */

        static std::vector<double> solveQuartic(double A, double B, double C, double D, double E);
    };
} // namespace RayTracer
