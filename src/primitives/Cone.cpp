/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Cone.hpp"

namespace RayTracer {

    Cone::Cone(const Math::Point3D& a, const Math::Vector3D& ax, double angDeg, double h, const Math::Vector3D& col)
      : Primitive(Material{"legacy", col, 0,0, {0,0,0}, 0,1}), apex(a), axis(ax / ax.length()),
        angle(angDeg * M_PI/180.0), height(h) {}
    
    Cone::Cone(const Math::Point3D& a, const Math::Vector3D& ax, double angDeg, double h, const Material& mat)
      : Primitive(mat), apex(a), axis(ax / ax.length()), angle(angDeg * M_PI/180.0), height(h) {}
    
    AABB Cone::bounds() const {
        Math::Point3D baseC = apex + axis * height;
        double   r     = height * std::tan(angle);
        Math::Vector3D e{ r * std::sqrt(1 - axis.x*axis.x),
                    r * std::sqrt(1 - axis.y*axis.y),
                    r * std::sqrt(1 - axis.z*axis.z) };
        Math::Point3D mn{ std::min(apex.x, baseC.x - e.x),
                    std::min(apex.y, baseC.y - e.y),
                    std::min(apex.z, baseC.z - e.z) };
        Math::Point3D mx{ std::max(apex.x, baseC.x + e.x),
                    std::max(apex.y, baseC.y + e.y),
                    std::max(apex.z, baseC.z + e.z) };
        return { mn, mx };
    }
    
    bool Cone::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        auto d = ray.direction;
        auto v = axis;
        auto o = ray.origin - apex;
        double cos2 = std::cos(angle) * std::cos(angle);

        double dv = d.dot(v);
        double ov = o.dot(v);
        double A = dv*dv - cos2 * d.dot(d);
        double B = 2*(dv*ov - cos2 * d.dot(o));
        double C = ov*ov - cos2 * o.dot(o);

        double disc = B*B - 4*A*C;
        double tSide = std::numeric_limits<double>::infinity();
        HitRecord recSide;

        if (disc >= 0) {
            double sdisc = std::sqrt(disc);
            for (double tc : {(-B - sdisc)/(2*A), (-B + sdisc)/(2*A)}) {
                if (tc <= EPS) continue;
                auto p = ray.origin + d * tc;
                double proj = (p - apex).dot(v);
                if (proj < 0 || proj > height) continue;
                if (tc < tSide) {
                    tSide = tc;
                    recSide.t     = tc;
                    recSide.point = p;
                    auto n = (p - apex) - v * proj;
                    recSide.normal = (n * cos2 - v * std::sqrt(n.dot(n)*(1 - cos2))) / std::sqrt(n.dot(n));
                    recSide.material = material;
                }
            }
        }

        double baseR = height * std::tan(angle);
        auto capC  = apex + axis * height;
        double denom = d.dot(v);
        double tCap  = std::numeric_limits<double>::infinity();
        HitRecord recCap;

        if (std::fabs(denom) > EPS) {
            double tc = (capC - ray.origin).dot(v) / denom;
            if (tc > EPS) {
                auto p = ray.origin + d * tc;
                if ((p - capC).dot(p - capC) <= baseR * baseR) {
                    tCap = tc;
                    recCap.t      = tc;
                    recCap.point  = p;
                    recCap.normal = v;
                    recCap.material = material;
                }
            }
        }

        bool hit = false;
        if (tSide < tCap) {
            tOut = recSide.t;
            rec  = recSide;
            hit = true;
        } else if (tCap < std::numeric_limits<double>::infinity()) {
            tOut = recCap.t;
            rec  = recCap;
            hit = true;
        }
        if (!hit) return false;

        if (rec.normal.dot(v) > 0.999) {
            Math::Vector3D uAxis, w = v;
            Math::Vector3D tangent = std::abs(w.x) > 0.9 ? Math::Vector3D{0,1,0} : Math::Vector3D{1,0,0};
            uAxis = w.cross(tangent); uAxis /= uAxis.length();
            Math::Vector3D vAxis = w.cross(uAxis);
            Math::Vector3D local = rec.point - capC;
            float u = std::fmod(local.dot(uAxis) * 1.0f, 1.0f);
            float vf= std::fmod(local.dot(vAxis) * 1.0f, 1.0f);
            if (u < 0) {
                u += 1;
            }
            if (vf < 0) {
                vf += 1;
            }
            rec.uv = {u, vf};
        } else {
            Math::Vector3D w = v;
            Math::Vector3D up = (std::abs(w.y) < 0.9) ? Math::Vector3D{0,1,0} : Math::Vector3D{1,0,0};
            Math::Vector3D uAxis = w.cross(up); uAxis /= uAxis.length();
            Math::Vector3D vAxis = w.cross(uAxis);
            Math::Vector3D local = rec.point - apex;
            double proj = local.dot(w);
            double theta = std::atan2(local.dot(vAxis), local.dot(uAxis));
            float u = static_cast<float>((theta + M_PI)/(2*M_PI));
            float vf = static_cast<float>(proj/height);
            rec.uv = {u, vf};
        }

        return true;
    }
};  // namespace RayTracer