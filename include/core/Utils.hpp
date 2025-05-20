/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <memory>
#include <omp.h>

#include "Raytracer.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Scene.hpp"
#include "rendering/BVH.hpp"
#include "loaders/SceneParser.hpp"
#include "rendering/ShadowShading.hpp"

namespace Utils {

    struct RenderParams {
        int W;
        int H;
        int spp;
        int max_depth;
        bool fast;
    };

    float clamp01(float v);

    Math::Point3D orbit(const Math::Point3D& center, double radius,
                               double yawDeg, double pitchDeg);

    bool checkUsage(int argc, char* argv[]);

    int parseThreads(int argc, char* argv[]);

    bool parseProgress(int argc, char* argv[]);

    RenderParams getRenderParams(int argc, char* argv[]);

    bool isDebugModeEnable(int argc, char* argv[]);

    void printSettings(const RenderParams& p, bool debug);

    bool loadSceneOrExit(const char* cfg, RayTracer::Scene& scene, bool debug);

    void PrintProgressBar(int percent, long long totalRays, const std::chrono::steady_clock::time_point &start);

    template<typename Ptr>
    inline std::vector<const RayTracer::Primitive*>
    collectPrimitives(const std::vector<Ptr>& objs)
    {
        std::vector<const RayTracer::Primitive*> ptrs;
        ptrs.reserve(objs.size());
        for (const auto& o : objs)
            ptrs.push_back(o.get());
        return ptrs;
    }

    template<typename Ptr>
    inline std::unique_ptr<RayTracer::BVHNode>
    buildBVH(const std::vector<Ptr>& objs)
    {
        auto ptrs = collectPrimitives(objs);
        return RayTracer::buildBVH(ptrs);
    }

    RayTracer::Camera createCamera(const RenderParams& p);

    std::vector<Math::Vector3D>
    renderImage(const RayTracer::Scene& scene,
                const RenderParams& p,
                bool showProgress);

    Math::Vector3D adjustContrast(const Math::Vector3D& c, float contrast);

    Math::Vector3D adjustSaturation(const Math::Vector3D& c, float satF);

    void postProcess(std::vector<Math::Vector3D>& fb, float contrast, float saturation);

    void writePPM(const std::vector<Math::Vector3D>& fb, int w, int h);

} // namespace Utils
