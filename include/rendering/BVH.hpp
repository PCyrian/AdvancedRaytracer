/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** BVH (8-way) Header
*/

#pragma once

#include <memory>
#include <vector>
#include <array>
#include <limits>
#include <immintrin.h>

#include "core/Ray.hpp"
#include "core/Primitive.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"

namespace RayTracer {

    using Float = double;

    struct AABB {
        Math::Point3D min;
        Math::Point3D max;

        AABB();
        AABB(const Math::Point3D& a, const Math::Point3D& b);

        bool hitFast(const Math::Point3D &orig,
                     const Float invDir[3],
                     const int dirNeg[3],
                     Float &tmin, Float &tmax) const;
    };

    AABB surrounding_box(const AABB &a, const AABB &b);

    struct RayPacket8 {
        std::array<float, 8> ox, oy, oz;
        std::array<float, 8> dx, dy, dz;
        std::array<float, 8> invDx, invDy, invDz;
        std::array<int, 8> negX, negY, negZ;
        std::array<float, 8> tMin, tMax;
        std::array<int, 8> valid;

        RayPacket8();
        void computeInverses();
    };

    class BVHNode {
    public:
        static constexpr int MAX_CHILDREN = 8;
        AABB box;
        std::array<std::unique_ptr<BVHNode>, MAX_CHILDREN> children;
        int childCount = 0;
        std::vector<const Primitive*> objects;  // leaf primitives

        std::array<float, MAX_CHILDREN> minX, minY, minZ;
        std::array<float, MAX_CHILDREN> maxX, maxY, maxZ;

        BVHNode(std::vector<const Primitive*>& objs);
        bool hit(const Ray &ray, Float &t, HitRecord &rec) const;
        void hitPacket(const RayPacket8& packet, std::array<bool, 8>& hits) const;
    };

    std::unique_ptr<BVHNode> buildBVH(std::vector<const Primitive*>& objs);

}  // namespace RayTracer
