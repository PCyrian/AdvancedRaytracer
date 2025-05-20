/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Sierpinski.hpp"

namespace RayTracer {

    Sierpinski::Sierpinski(const Math::Point3D& center, double size, int depth, const Material& mat)
        : Primitive(mat) {
        generate(center, size, depth);
    }

    bool Sierpinski::hits(const Ray& ray, double& tHit, HitRecord& rec) const
    {
        bool hit = false;
        double closest = std::numeric_limits<double>::max();

        for (const auto& child : _children) {
            double t;
            HitRecord tempRec;
            if (child->hits(ray, t, tempRec) && t < closest) {
                closest = t;
                tHit = t;
                rec = tempRec;
                hit = true;
            }
        }

        return hit;
    }

    AABB Sierpinski::bounds() const
    {
        if (_children.empty())
            return AABB{};

        AABB result = _children.front()->bounds();
        for (size_t i = 1; i < _children.size(); ++i)
            result = surrounding_box(result, _children[i]->bounds());
        return result;
    }

    void Sierpinski::generate(const Math::Point3D& center, double size, int depth)
    {
        if (depth == 0) {
            _children.push_back(std::make_shared<Tetrahedron>(center, size, material));
            return;
        }

        double half = size / 2.0;

        Math::Vector3D offsets[] = {
            { 0, 0, 0 },
            { half, 0, 0 },
            { half / 2.0, std::sqrt(3.0) * half / 2.0, 0 },
            { half / 2.0, std::sqrt(3.0) * half / 6.0, std::sqrt(6.0) * half / 3.0 }
        };

        for (const auto& offset : offsets) {
            Math::Point3D newCenter = center + offset;
            _children.push_back(std::make_shared<Sierpinski>(newCenter, half, depth - 1, material));
        }
    }
}; // namespace RayTracer
