/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include <immintrin.h>
#include <algorithm>
#include <cstring>

#include "../../include/rendering/BVH.hpp"
#include "../../include/core/SimdAABB.hpp"

namespace RayTracer {

AABB::AABB() = default;
AABB::AABB(const Math::Point3D& a, const Math::Point3D& b)
    : min(a), max(b) {}

bool AABB::hitFast(const Math::Point3D &orig, const Float invDir[3], const int dirNeg[3], Float &tmin, Float &tmax) const {
    //scalar slab intersection
    Float t0 = ((dirNeg[0] ? max.x : min.x) - orig.x) * invDir[0];
    Float t1 = ((dirNeg[0] ? min.x : max.x) - orig.x) * invDir[0];
    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;
    if (tmax <= tmin) return false;

    t0 = ((dirNeg[1] ? max.y : min.y) - orig.y) * invDir[1];
    t1 = ((dirNeg[1] ? min.y : max.y) - orig.y) * invDir[1];
    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;
    if (tmax <= tmin) return false;

    t0 = ((dirNeg[2] ? max.z : min.z) - orig.z) * invDir[2];
    t1 = ((dirNeg[2] ? min.z : max.z) - orig.z) * invDir[2];
    tmin = t0 > tmin ? t0 : tmin;
    tmax = t1 < tmax ? t1 : tmax;
    return tmax > tmin;
}

AABB surrounding_box(const AABB &a, const AABB &b) {
    Math::Point3D small{ std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z) };
    Math::Point3D big  { std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z) };
    return AABB(small, big);
}

RayPacket8::RayPacket8() {
    tMin.fill(1e-6f);
    tMax.fill(1e30f);
    valid.fill(1);
}

void RayPacket8::computeInverses() {
    for (int i = 0; i < 8; ++i) {
        invDx[i] = 1.0f / dx[i];
        invDy[i] = 1.0f / dy[i];
        invDz[i] = 1.0f / dz[i];
        negX[i]  = invDx[i] < 0;
        negY[i]  = invDy[i] < 0;
        negZ[i]  = invDz[i] < 0;
    }
}

BVHNode::BVHNode(std::vector<const Primitive*>& objs) {
    if (objs.empty()) return;
    constexpr Float kEps = 1e-4;
    box = objs.front()->bounds();
    box.min.x -= kEps; box.min.y -= kEps; box.min.z -= kEps;
    box.max.x += kEps; box.max.y += kEps; box.max.z += kEps;

    for (size_t i = 1; i < objs.size(); ++i)
        box = surrounding_box(box, objs[i]->bounds());

    if (objs.size() <= 2) {
        objects = objs;
        childCount = 0;
    } else {
        Math::Vector3D span = box.max - box.min;
        int axis = (span.x > span.y && span.x > span.z) ? 0 : (span.y > span.z ? 1 : 2);
        std::sort(objs.begin(), objs.end(), [&](const Primitive* a, const Primitive* b) {
            auto ba = a->bounds(), bb = b->bounds();
            Float va = (axis==0? ba.min.x : axis==1? ba.min.y : ba.min.z);
            Float vb = (axis==0? bb.min.x : axis==1? bb.min.y : bb.min.z);
            return va < vb;
        });
        size_t total = objs.size(), base = total / MAX_CHILDREN, rem = total % MAX_CHILDREN, off = 0;
        childCount = 0;
        for (int i = 0; i < MAX_CHILDREN && off < total; ++i) {
            size_t cnt = base + (i < (int)rem ? 1 : 0);
            std::vector<const Primitive*> slice(objs.begin()+off, objs.begin()+off+cnt);
            children[childCount++] = std::make_unique<BVHNode>(slice);
            off += cnt;
        }
        objects.clear();
        for (int i = 0; i < childCount; ++i) {
            auto& c = children[i]->box;
            minX[i] = (float)c.min.x; minY[i] = (float)c.min.y; minZ[i] = (float)c.min.z;
            maxX[i] = (float)c.max.x; maxY[i] = (float)c.max.y; maxZ[i] = (float)c.max.z;
        }
    }
}

bool BVHNode::hit(const Ray &ray, Float &t, HitRecord &rec) const {
    constexpr Float kEps = 1e-6;
    bool hitAny = false;
    Float closest = t;
    HitRecord bestRec;

    Float invDir[3] = {1/ray.direction.x,1/ray.direction.y,1/ray.direction.z};
    int dirNeg[3] = {invDir[0]<0,invDir[1]<0,invDir[2]<0};
    const BVHNode* stack[64]; int sp = 0; stack[sp++] = this;

    while (sp) {
        const BVHNode* node = stack[--sp];
        Float tmin = kEps, tmax = closest;
        if (!node->box.hitFast(ray.origin, invDir, dirNeg, tmin, tmax)) continue;
        if (node->childCount > 0) {
            for (int i = 0; i < node->childCount; ++i)
                stack[sp++] = node->children[i].get();
        } else {
            for (auto o : node->objects) {
                Float tObj; HitRecord tmp;
                if (o->hits(ray, tObj, tmp) && tObj < closest) {
                    hitAny = true; closest = tObj; bestRec = tmp; bestRec.t = tObj;
                }
            }
        }
    }
    if (hitAny) { t = closest; rec = bestRec; }
    return hitAny;
}

void BVHNode::hitPacket(const RayPacket8& packet, std::array<bool,8>& hits) const {
    hits.fill(false);
    // load packet
    __m256 oX = _mm256_loadu_ps(packet.ox.data()), oY = _mm256_loadu_ps(packet.oy.data()), oZ = _mm256_loadu_ps(packet.oz.data());
    __m256 iX = _mm256_loadu_ps(packet.invDx.data()), iY = _mm256_loadu_ps(packet.invDy.data()), iZ = _mm256_loadu_ps(packet.invDz.data());
    __m256i nX = _mm256_loadu_si256((__m256i*)packet.negX.data()), nY = _mm256_loadu_si256((__m256i*)packet.negY.data()), nZ = _mm256_loadu_si256((__m256i*)packet.negZ.data());
    __m256 tmin = _mm256_loadu_ps(packet.tMin.data()), tmax = _mm256_loadu_ps(packet.tMax.data());

    // pack into Simd AABB8 via constructor
    Simd::AABB8 aabb(minX.data(), minY.data(), minZ.data(),
                     maxX.data(), maxY.data(), maxZ.data());
    __m256i mask = Simd::hitFast8(oX, oY, oZ, iX, iY, iZ, nX, nY, nZ, aabb, tmin, tmax);
    unsigned mov = _mm256_movemask_ps(_mm256_castsi256_ps(mask));

    for (int i = 0; i < 8; ++i) {
        if (!(mov & (1u << i))) continue;
        if (childCount > 0) for (int c = 0; c < childCount; ++c)
            children[c]->hitPacket(packet, hits);
        else {
            Ray ray{ {packet.ox[i],packet.oy[i],packet.oz[i]}, {packet.dx[i],packet.dy[i],packet.dz[i]} };
            for (auto prim : objects) {
                Float tObj; HitRecord rec;
                if (prim->hits(ray, tObj, rec)) hits[i] = true;
            }
        }
    }
}

std::unique_ptr<BVHNode> buildBVH(std::vector<const Primitive*>& objs) {
    return std::make_unique<BVHNode>(objs);
}

} // namespace RayTracer