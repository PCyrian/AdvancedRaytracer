/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <vector>
#include <memory>
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"
#include "rendering/Material.hpp"
#include "Triangle.hpp"

namespace RayTracer {

    /**
     * @class Tetrahedron
     * @brief Primitive composée de 4 triangles formant un tétraèdre.
     */
    class Tetrahedron : public Primitive {
    public:
        /**
         * @brief Constructeur
         * @param center Centre du tétraèdre
         * @param size   Taille (longueur d'une arête)
         * @param mat    Matériau
         */
        Tetrahedron(const Math::Point3D& center, double size, const Material& mat);
        bool hits(const Ray& ray, double& tHit, HitRecord& rec) const override;
        AABB bounds() const override;

    private:
        std::vector<std::shared_ptr<Primitive>> _faces;
        void generate(const Math::Point3D& center, double size, const Material& mat);
    };

} // namespace RayTracer
