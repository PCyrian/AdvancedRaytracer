/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/FASTRenderer.hpp"
#include "../../include/rendering/ShadowShading.hpp"
#include "../../include/rendering/Skybox.hpp"
#include "../../include/core/Utils.hpp"

#include <vector>

#include <vector>
#include <cmath>
#include <chrono>


namespace Utils {

    static inline double rand01() {
        return rand() / (RAND_MAX + 1.0);
    }

    Math::Vector3D sampleHemisphere(const Math::Vector3D &N) {
        double r1 = rand01(), r2 = rand01();
        double phi = 2 * M_PI * r1;
        double cosTheta = r2;
        double sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
        Math::Vector3D local{
            sinTheta * std::cos(phi),
            sinTheta * std::sin(phi),
            cosTheta
        };
        // Build tangent space
        Math::Vector3D up = (std::abs(N.z) < 0.999) ? Math::Vector3D{0,0,1} : Math::Vector3D{1,0,0};
        Math::Vector3D tangent = N.cross(up).normalized();
        Math::Vector3D bitangent = N.cross(tangent);
        return tangent * local.x + bitangent * local.y + N * local.z;
    }

    Math::Vector3D FastShade(const RayTracer::Ray &ray, const RayTracer::Scene &scene) {
        const double EPS = 1e-4;
        double tHit;
        RayTracer::HitRecord rec;
        if (!scene.hit(ray, tHit, rec)) {
            // Simple flat background
            return {0.2, 0.3, 0.4};
        }

        // Base color
        Math::Vector3D albedo = rec.material.diffuseMap
            ? rec.material.diffuseMap->sample(rec.uv.x, rec.uv.y)
            : rec.material.albedo;

        // Ambient Occlusion
        const int aoSamples = 4;
        int occluded = 0;
        for (int i = 0; i < aoSamples; ++i) {
            Math::Vector3D dir = sampleHemisphere(rec.normal);
            RayTracer::Ray aoRay{rec.point + rec.normal * EPS, dir};
            double t2;
            RayTracer::HitRecord hr;
            if (scene.hit(aoRay, t2, hr)) {
                occluded++;
            }
        }
        double ao = 1.0 - (static_cast<double>(occluded) / aoSamples);

        return albedo * ao;
    }

    std::vector<Math::Vector3D> FASTRenderer(const RayTracer::Scene &scene,
                                             const Utils::RenderParams &p,
                                             bool showProgress) {
        int w = p.W, h = p.H, spp = std::max(1, p.spp);
        long long totalRays = 1LL * w * h * spp;
        std::vector<Math::Vector3D> fb(w * h);
        double inv_spp = 1.0 / spp;

        using clk = std::chrono::steady_clock;
        auto start = clk::now();
        int next_pct = 1;

        for (int j = 0; j < h; ++j) {
    #pragma omp parallel for schedule(static)
            for (int i = 0; i < w; ++i) {
                Math::Vector3D col{0, 0, 0};
                for (int s = 0; s < spp; ++s) {
                    double u = (i + 0.5) / (w - 1);
                    double v = (j + 0.5) / (h - 1);
                    RayTracer::Ray ray = scene.camera.ray(u, 1.0 - v);
                    col += FastShade(ray, scene);
                }
                fb[j * w + i] = col * inv_spp;
            }
            if (showProgress) {
                int pct = (j + 1) * 100 / h;
                while (pct >= next_pct && next_pct <= 100) {
                    PrintProgressBar(next_pct++, totalRays, start);
                }
            }
        }
        if (showProgress) std::cerr << "\n";
        return fb;
    }

} // namespace Utils
