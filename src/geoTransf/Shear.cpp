/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/geoTransf/Shear.hpp"
#include <algorithm>
#include <cmath>
#include <cassert>

namespace RayTracer {

    static inline Math::Vector3D mul33(const double m[3][3], const Math::Vector3D& v)
    {
        return {
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        };
    }

    static inline Math::Point3D mul33(const double m[3][3], const Math::Point3D& p)
    {
        return {
            m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z,
            m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z,
            m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z
        };
    }

    Shear::Shear(std::shared_ptr<Primitive> prim, double sXY, double sXZ, double sYX,
        double sYZ,double sZX,double sZY)
        : Primitive(prim->getMaterial()), _prim(std::move(prim))
    {
        _m[0][0]=1;   _m[0][1]=sXY; _m[0][2]=sXZ;
        _m[1][0]=sYX; _m[1][1]=1;   _m[1][2]=sYZ;
        _m[2][0]=sZX; _m[2][1]=sZY; _m[2][2]=1;

        double d =
            _m[0][0]*(_m[1][1]*_m[2][2]-_m[1][2]*_m[2][1])
            - _m[0][1]*(_m[1][0]*_m[2][2]-_m[1][2]*_m[2][0])
            + _m[0][2]*(_m[1][0]*_m[2][1]-_m[1][1]*_m[2][0]);
        assert(std::fabs(d) > 1e-9 && "singular shear matrix");
        double invDet = 1.0/d;

        _mInv[0][0] =  (_m[1][1]*_m[2][2]-_m[1][2]*_m[2][1])*invDet;
        _mInv[0][1] = -(_m[0][1]*_m[2][2]-_m[0][2]*_m[2][1])*invDet;
        _mInv[0][2] =  (_m[0][1]*_m[1][2]-_m[0][2]*_m[1][1])*invDet;
        _mInv[1][0] = -(_m[1][0]*_m[2][2]-_m[1][2]*_m[2][0])*invDet;
        _mInv[1][1] =  (_m[0][0]*_m[2][2]-_m[0][2]*_m[2][0])*invDet;
        _mInv[1][2] = -(_m[0][0]*_m[1][2]-_m[0][2]*_m[1][0])*invDet;
        _mInv[2][0] =  (_m[1][0]*_m[2][1]-_m[1][1]*_m[2][0])*invDet;
        _mInv[2][1] = -(_m[0][0]*_m[2][1]-_m[0][1]*_m[2][0])*invDet;
        _mInv[2][2] =  (_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0])*invDet;
        for(int i=0;i<3;++i)
            for(int j=0;j<3;++j)
                _nMat[i][j] = _mInv[j][i];
    }

    bool Shear::hits(const Ray& r, double& t, HitRecord& rec) const
    {
    Ray rl;

    rl.origin    = mul33(_mInv, r.origin);
    rl.direction = mul33(_mInv, r.direction).normalized();
    if (!_prim->hits(rl, t, rec))
        return false;
    rec.point  = mul33(_m, rec.point);
    rec.normal = mul33(_nMat, rec.normal).normalized();
    return true;
    }

    AABB Shear::bounds() const
    {
    const AABB b = _prim->bounds();

    Math::Point3D c[8] = { {b.min.x,b.min.y,b.min.z}, {b.max.x,b.min.y,b.min.z},
                     {b.min.x,b.max.y,b.min.z}, {b.max.x,b.max.y,b.min.z},
                     {b.min.x,b.min.y,b.max.z}, {b.max.x,b.min.y,b.max.z},
                     {b.min.x,b.max.y,b.max.z}, {b.max.x,b.max.y,b.max.z} };

    Math::Point3D mn = mul33(_m, c[0]), mx = mn;
    for (int i=1;i<8;++i) {
        Math::Point3D p = mul33(_m, c[i]);
        mn.x = std::min(mn.x, p.x);  mx.x = std::max(mx.x, p.x);
        mn.y = std::min(mn.y, p.y);  mx.y = std::max(mx.y, p.y);
        mn.z = std::min(mn.z, p.z);  mx.z = std::max(mx.z, p.z);
        }
        return {mn, mx};
    }
} //namespace RayTracer
