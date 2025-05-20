/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <vector>
#include <memory>
#include <limits>
#include <cmath>
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "Tetrahedron.hpp"

namespace RayTracer {

    /**
     * @class Sierpinski
     * @brief Représente un tétraèdre de Sierpinski récursif.
     */
    class Sierpinski : public Primitive {
    public:
        Sierpinski(const Math::Point3D& center, double size, int depth, const Material& mat);
        bool hits(const Ray& ray, double& tHit, HitRecord& rec) const override;
        AABB bounds() const override;

    private:
        std::vector<std::shared_ptr<Primitive>> _children;
        void generate(const Math::Point3D& center, double size, int depth);
    };

} // namespace RayTracer
