/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <vector>
#include "core/Vector3D.hpp"
#include "rendering/Scene.hpp"
#include "core/Utils.hpp"

namespace Viz {
    enum class VizMode {
        DEFAULT,
        SCANLINE,
        TILES,
        RANDOM
    };

#ifdef USE_SFML
    /**
     * @brief Render the scene into an SFML window and return the framebuffer.
     * @param scene      The scene to ray-trace.
     * @param p          Rendering parameters.
     * @param mode       Which VizMode to use.
     * @param showProgress  Whether to display Utils::PrintProgressBar.
     * @returns A flat vector of Vector3D pixels.
     */
    std::vector<Math::Vector3D>
    renderImageSFML(const RayTracer::Scene& scene,
                    const Utils::RenderParams& p,
                    VizMode mode,
                    bool showProgress);
#endif
    VizMode parseVizMode(int argc, char* argv[]);
} // namespace Viz
