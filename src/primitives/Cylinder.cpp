/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Cylinder.hpp"

namespace RayTracer {

    Cylinder::Cylinder(const Math::Point3D& b, const Math::Vector3D& ax, double r, double h, const Math::Vector3D& col)
    : Primitive(Material{"legacy", col, 0.0f, 0.0f, Math::Vector3D{0,0,0}, 0.0f, 1.0f}), base(b), axis(ax / ax.length()),
        radius(r), height(h) {}

    Cylinder::Cylinder(const Math::Point3D& b, const Math::Vector3D& ax, double r, double h, const Material& mat)
    : Primitive(mat), base(b), axis(ax / ax.length()), radius(r), height(h){}

    AABB Cylinder::bounds() const
    {
        Math::Point3D top = base + axis * height;
        Math::Vector3D extents{
            radius * std::sqrt(1 - axis.x*axis.x),
            radius * std::sqrt(1 - axis.y*axis.y),
            radius * std::sqrt(1 - axis.z*axis.z)
        };
        Math::Point3D mn{
            std::min(base.x, top.x) - extents.x,
            std::min(base.y, top.y) - extents.y,
            std::min(base.z, top.z) - extents.z
        };
        Math::Point3D mx{
            std::max(base.x, top.x) + extents.x,
            std::max(base.y, top.y) + extents.y,
            std::max(base.z, top.z) + extents.z
        };
        return { mn, mx };
    }

    bool Cylinder::hits(const Ray& ray, double& tOut, HitRecord& rec) const
    {
        const double EPS = 1e-8;
        auto d     = ray.direction;
        auto o     = ray.origin - base;
        double d_dot = d.dot(axis);
        double o_dot = o.dot(axis);
        auto d_perp  = d - axis * d_dot;
        auto o_perp  = o - axis * o_dot;

        double A = d_perp.dot(d_perp);
        double B = 2 * o_perp.dot(d_perp);
        double C = o_perp.dot(o_perp) - radius*radius;
        double disc = B*B - 4*A*C;
        if (disc < 0) return false;

        double sqrtD = std::sqrt(disc);
        double t1 = (-B - sqrtD) / (2*A);
        double t2 = (-B + sqrtD) / (2*A);
        double t  = (t1 > EPS ? t1 : (t2 > EPS ? t2 : -1));
        if (t < EPS) return false;

        Math::Point3D p = ray.origin + d * t;
        double proj = (p - base).dot(axis);
        bool isSide = (proj >= -EPS && proj <= height + EPS);

        double tCap  = std::numeric_limits<double>::infinity();
        bool   capH  = false;
        Math::Vector3D capN;

        if (std::abs(d_dot) > EPS) {
            double tc0 = -o_dot / d_dot;
            if (tc0 > EPS) {
                Math::Point3D p0 = ray.origin + d * tc0;
                if ((p0 - base).dot(p0 - base) <= radius*radius) {
                    tCap = tc0; capH = true; capN = axis * -1.0;
                }
            }
            double tc1 = (height - o_dot) / d_dot;
            if (tc1 > EPS) {
                Math::Point3D p1 = ray.origin + d * tc1;
                if ((p1 - (base + axis*height)).dot(p1 - (base + axis*height)) <= radius*radius) {
                    if (tc1 < tCap) {
                        tCap = tc1; capH = true; capN = axis;
                    }
                }
            }
        }

        if (!isSide && !capH) return false;

        if (capH && (!isSide || tCap < t)) {
            t        = tCap;
            rec.normal = capN;
            rec.point  = ray.origin + d * t;
        } else {
            rec.point  = p;
            Math::Vector3D n = (p - base) - axis * proj;
            rec.normal = n / n.length();
        }

        rec.t        = t;
        tOut         = t;
        rec.material = material;

        Math::Vector3D w    = axis;
        Math::Vector3D up   = (std::abs(w.y) < 0.9) ? Math::Vector3D{0,1,0} : Math::Vector3D{1,0,0};
        Math::Vector3D uAx  = w.cross(up);  uAx = uAx / uAx.length();
        Math::Vector3D vAx  = w.cross(uAx);

        Math::Vector3D local = rec.point - base;
        double theta   = std::atan2(local.dot(vAx), local.dot(uAx));
        float u        = static_cast<float>((theta + M_PI) / (2.0 * M_PI));
        float v        = static_cast<float>(proj / height);
        rec.uv         = { u, v };

        return true;
    }

} // namespace RayTracer