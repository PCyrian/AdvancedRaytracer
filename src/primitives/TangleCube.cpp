/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/TangleCube.hpp"

namespace RayTracer {

    TangleCube::TangleCube(const Math::Point3D& center, double scale, const Material& mat)
    : Primitive(mat), _center(center), _scale(scale) {}

    TangleCube::~TangleCube() = default;

    bool TangleCube::hits(const Ray& ray, double& tHit, HitRecord& rec) const
    {
        Math::Vector3D O = ray.origin - _center;
        Math::Vector3D D = ray.direction;
        constexpr double alpha = 5.0, beta = 11.8;
        double Ox = O.x, Dx = D.x;
        double Oy = O.y, Dy = D.y;
        double Oz = O.z, Dz = D.z;
        double A = Dx*Dx*Dx*Dx + Dy*Dy*Dy*Dy + Dz*Dz*Dz*Dz;
        double B = 4*(Ox*Dx*Dx*Dx + Oy*Dy*Dy*Dy + Oz*Dz*Dz*Dz);
        double C = 6*(Ox*Ox*Dx*Dx + Oy*Oy*Dy*Dy + Oz*Oz*Dz*Dz)
                - alpha*(Dx*Dx + Dy*Dy + Dz*Dz);
        double D_ = 4*(Ox*Ox*Ox*Dx + Oy*Oy*Oy*Dy + Oz*Oz*Oz*Dz)
                - 2*alpha*(Ox*Dx + Oy*Dy + Oz*Dz);
        double E = Ox*Ox*Ox*Ox + Oy*Oy*Oy*Oy + Oz*Oz*Oz*Oz
                - alpha*(Ox*Ox + Oy*Oy + Oz*Oz)
                + beta;
        auto roots = solveQuartic(A, B, C, D_, E);
        double best = std::numeric_limits<double>::infinity();

        for (double t : roots) {
            if (t > 1e-4 && t < best)
                best = t;
        }
        if (!std::isfinite(best))
            return false;
        tHit = best;
        Math::Point3D P = ray.origin + D * tHit;
        Math::Vector3D Q = P - _center;
        double x = Q.x, y = Q.y, z = Q.z;
        Math::Vector3D N(
            4*x*x*x - 2*alpha*x,
            4*y*y*y - 2*alpha*y,
            4*z*z*z - 2*alpha*z
        );
        if (double len = N.length(); len > 0)
            N = N * (1.0/len);
        rec.point    = P;
        rec.normal   = N;
        rec.material = material;
        rec.color    = material.albedo;
        return true;
    }

    AABB TangleCube::bounds() const
    {
        double pad = _scale + 1.0;
        Math::Vector3D r(pad, pad, pad);
        return { _center - r, _center + r };
    }

    std::vector<double> TangleCube::solveQuartic(double A, double B, double C, double D, double E)
    {
        using cd = std::complex<double>;
        const double tol = 1e-8;
        const int maxIter = 1000;
        std::vector<cd> complexRoots(4);

        complexRoots[0] = cd(1.0, 0.0);
        complexRoots[1] = cd(0.0, 1.0);
        complexRoots[2] = cd(-1.0, 0.0);
        complexRoots[3] = cd(0.0, -1.0);
        for (int iter = 0; iter < maxIter; iter++) {
            bool done = true;
            for (int i = 0; i < 4; i++) {
                cd currentRoot = complexRoots[i];
                cd polyValue = (((cd(A) * currentRoot + cd(B)) * currentRoot + cd(C)) * currentRoot + cd(D)) * currentRoot + cd(E);
                cd denominator = 1.0;
                for (int j = 0; j < 4; j++) {
                    if (j != i)
                        denominator *= (currentRoot - complexRoots[j]);
                }
                if (std::abs(denominator) < tol)
                    continue;
                cd delta = polyValue / denominator;
                if (std::abs(delta) > tol)
                    done = false;
                complexRoots[i] = currentRoot - delta;
            }
            if (done)
                break;
        }
        std::vector<double> realRoots;
        for (auto &z : complexRoots) {
            if (std::abs(z.imag()) < 1e-6) {
                realRoots.push_back(z.real());
            }
        }
        return realRoots;
    }
} // namespace RayTracer
    