/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Shear.hpp
 * @brief Declares the geometric transformation shear.
 * @author Thibaut Louis
 * @version 1.0
 * @date 2025-05
 *
 * @details
*/
#pragma once

#include <memory>
#include "core/Primitive.hpp"
#include "rendering/BVH.hpp"

namespace RayTracer {

    class Shear final : public Primitive {
        public :
            Shear(std::shared_ptr<Primitive> prim,
            double sXY = 0, double sXZ = 0,
            double sYX = 0, double sYZ = 0,
            double sZX = 0, double sZY = 0);

        bool hits(const Ray& r, double& t, HitRecord& rec)const override;
        AABB bounds() const override;

        private:
            std::shared_ptr<Primitive> _prim;
            double _m [3][3];
            double _mInv[3][3];
            double _nMat[3][3];
    };
} // namespace RayTracer
