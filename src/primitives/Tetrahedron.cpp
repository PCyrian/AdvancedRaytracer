/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Tetrahedron.hpp"

namespace RayTracer {

    Tetrahedron::Tetrahedron(const Math::Point3D& center, double size, const Material& mat) : Primitive(mat)
    {
        generate(center, size, mat);
    }

    bool Tetrahedron::hits(const Ray& ray, double& tHit, HitRecord& rec) const
    {
        bool hit = false;
        double closest = std::numeric_limits<double>::max();

        for (const auto& face : _faces) {
            double t;
            HitRecord tempRec;
            if (face->hits(ray, t, tempRec) && t < closest) {
                tHit = t;
                rec = tempRec;
                closest = t;
                hit = true;
            }
        }

        return hit;
    }

    AABB Tetrahedron::bounds() const
    {
        if (_faces.empty())
            return AABB{};

        AABB result = _faces.front()->bounds();
        for (size_t i = 1; i < _faces.size(); ++i)
            result = surrounding_box(result, _faces[i]->bounds());
        return result;
    }

    void Tetrahedron::generate(const Math::Point3D& center, double size, const Material& mat)
    {
        Math::Point3D A = center + Math::Vector3D(0, 0, 0);
        Math::Point3D B = center + Math::Vector3D(size, 0, 0);
        Math::Point3D C = center + Math::Vector3D(size / 2, std::sqrt(3.0) * size / 2, 0);
        Math::Point3D D = center + Math::Vector3D(size / 2, std::sqrt(3.0) * size / 6, std::sqrt(6.0) * size / 3);

        _faces.push_back(std::make_shared<Triangle>(A, B, C, mat));
        _faces.push_back(std::make_shared<Triangle>(A, B, D, mat));
        _faces.push_back(std::make_shared<Triangle>(A, C, D, mat));
        _faces.push_back(std::make_shared<Triangle>(B, C, D, mat));
    }
}; // namespace RayTracer
