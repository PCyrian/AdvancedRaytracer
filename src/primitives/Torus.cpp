/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/primitives/Torus.hpp"

namespace RayTracer {

    Torus::Torus(const Math::Point3D& c,
            double majorRadius,
            double minorRadius,
            const Material& mat)
        : Primitive(mat), center(c), R(majorRadius), r(minorRadius) {}

    bool Torus::hits(const Ray& ray, double& tHit, HitRecord& rec) const
    {
        Math::Vector3D O = ray.origin - center;
        Math::Vector3D D = ray.direction;
        constexpr double tMinSearch = 1e-4;
        constexpr double tMaxSearch = 100.0;
        double t;

        if (!intersectTorus(O, D, R, r, tMinSearch, tMaxSearch, t))
            return false;
        if (!intersectTorus(O, D, R, r, tMinSearch, tMaxSearch, t))
            return false;

        tHit = t;
        Math::Point3D P = ray.origin + ray.direction * tHit;
        Math::Vector3D Q = P - center;
        double sumQ2 = Q.dot(Q);
        Math::Vector3D N = Q * (4.0 * (sumQ2 - R*R - r*r))
                            + Math::Vector3D(0.0, 8.0 * R * R * Q.y, 0.0);
        double lenN = N.length();
        if (lenN > 0.0) N = N * (1.0 / lenN);
        rec.point    = P;
        rec.normal   = N;

        double theta = std::atan2(Q.z, Q.x);
        float u = static_cast<float>((theta + M_PI) / (2.0 * M_PI));
        double lenXZ = std::sqrt(Q.x*Q.x + Q.z*Q.z);
        double phi = std::atan2(Q.y, lenXZ - R);
        float v = static_cast<float>((phi + M_PI) / (2.0 * M_PI));

        u = std::fmod(u, 1.0f); if (u < 0.0f) u += 1.0f;
        v = std::fmod(v, 1.0f); if (v < 0.0f) v += 1.0f;
        rec.uv = { u, v };

        rec.material = material;
        rec.color    = material.albedo;
        return true;
    }

    AABB Torus::bounds() const
    {
        Math::Point3D bb_min = center + Math::Vector3D(-(R + r), -r, -(R + r));
        Math::Point3D bb_max = center + Math::Vector3D( (R + r),  r,  (R + r));

        return AABB{ bb_min, bb_max };
    }

    double Torus::torusField(const Math::Vector3D& O, const Math::Vector3D& D, double t,
        double majorR, double minorR) 
    {
        Math::Vector3D P = O + D * t;
        double x2 = P.x*P.x, y2 = P.y*P.y, z2 = P.z*P.z;
        double sum = x2 + y2 + z2 + majorR*majorR - minorR*minorR;

        return sum*sum - 4.0*majorR*majorR*(x2 + z2);
    }

    bool Torus::intersectTorus(const Math::Vector3D& O, const Math::Vector3D& D, double R, double r,
        double tMinSearch, double tMaxSearch, double& tHit)
    {
        const int steps = 1000;
        double stepSize = (tMaxSearch - tMinSearch) / static_cast<double>(steps);
        double prevT = tMinSearch;
        double prevField = torusField(O, D, prevT, R, r);
    
        for (int i = 1; i <= steps; i++) {
            double currentT = tMinSearch + i * stepSize;
            double currentField = torusField(O, D, currentT, R, r);
            if (prevField * currentField <= 0.0) {
                for (int it = 0; it < 40; it++) {
                    double m  = 0.5 * (prevT + currentT);
                    double fm = torusField(O, D, m, R, r);
                    if (prevField * fm <= 0.0) {
                        currentT = m;
                    } else {
                        prevT  = m;
                        prevField = fm;
                    }
                }
                tHit = 0.5 * (prevT + currentT);
                return true;
            }
            prevT = currentT;
            prevField = currentField;
        }
        return false;
    }

    double Torus::solveCubicReal(double A, double B, double C, double D)
    {
        const double eps = 1e-12;

        if (std::fabs(A) < eps) {
            if (std::fabs(B) < eps) {
                return (std::fabs(C) > eps) ? -D / C : 0.0;
            }
            double disc = C*C - 4.0 * B * D;
            if (disc >= 0.0) {
                double sqrtDisc = std::sqrt(disc);
                double r1 = (-C + sqrtDisc) / (2.0 * B);
                double r2 = (-C - sqrtDisc) / (2.0 * B);
                return (r1 >= 0.0) ? r1 : r2;
            }
            return -C / (2.0 * B);
        }
        double a = B/A;
        double b = C/A;
        double c = D/A;
        double a2 = a * a;
        double p = (3.0*b - a2) / 3.0;
        double q = (2.0*a*a2 - 9.0*a*b + 27.0*c) / 27.0;
        double disc = q*q/4.0 + p*p*p/27.0;
        double shift = a / 3.0;
        if (disc >= 0.0) {
            double sqrtDisc = std::sqrt(disc);
            double u = std::cbrt(-q/2.0 + sqrtDisc);
            double v = std::cbrt(-q/2.0 - sqrtDisc);
            return u + v - shift;
        } else {
            double rho = std::sqrt(-p*p*p/27.0);
            double theta = std::acos(-q/(2.0 * rho));
            double x = 2.0 * std::cbrt(rho) * std::cos(theta / 3.0);
            return x - shift;
        }
    }

    std::vector<double> Torus::solveQuadratic(double a, double b, double c)
    {
        std::vector<double> roots;

        if (std::fabs(a) < EPS) {
            if (std::fabs(b) > EPS)
                roots.push_back(-c/b);
            return roots;
        }
        double delta = b*b - 4*a*c;
        if (delta < -EPS) return roots;
        if (delta < 0) delta = 0;
        double sq = std::sqrt(delta);
        roots.push_back((-b + sq) / (2*a));
        roots.push_back((-b - sq) / (2*a));
        return roots;
    }

    std::vector<double> Torus::solveCubicAllReal(double a, double b, double c, double d)
    {
        std::vector<double> roots;

        if (std::fabs(a) < EPS)
            return solveQuadratic(b, c, d);

        double A = b/a, B = c/a, C = d/a;
        double offset = A/3.0;
        double p = B - A*A/3.0;
        double q = 2*A*A*A/27.0 - A*B/3.0 + C;
        double discr = (q/2.0)*(q/2.0) + (p/3.0)*(p/3.0)*(p/3.0);

        if (discr >= -EPS) {
            double sqrt_disc = std::sqrt(std::max(0.0, discr));
            double u = std::cbrt(-q/2.0 + sqrt_disc);
            double v = std::cbrt(-q/2.0 - sqrt_disc);
            roots.push_back(u + v - offset);
        } else {
            double radius = std::sqrt(-p/3.0);
            double cosArg = std::clamp(-q/(2.0*radius*radius*radius), -1.0, 1.0);
            double angle = std::acos(cosArg);
            roots.push_back( 2*radius*std::cos(angle/3.0) - offset );
            roots.push_back( 2*radius*std::cos((angle + 2*PI)/3.0) - offset );
            roots.push_back( 2*radius*std::cos((angle + 4*PI)/3.0) - offset );
        }
        return roots;
    }

    std::vector<double> Torus::solveQuartic(double A, double B, double C, double D, double E)
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
};// namespace RayTracer
