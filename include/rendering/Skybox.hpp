/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Skybox
*/

#pragma once

#include "core/Vector3D.hpp"
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 (M_PI / 2.0)
#endif

#define SKYBOX_TEXTURE_SCALE 1.0

namespace Skybox {

    struct PPMImage {
        int width = 0;
        int height = 0;
        std::vector<unsigned char> data;

        bool isLoaded() const;
        Math::Vector3D getColorAtTexel(int x, int y) const;
        Math::Vector3D sample(double u, double v) const;
    };

    namespace detail {
        extern PPMImage globalSkyboxTexture;
    }

    bool loadPPMFile(const std::string& filepath, PPMImage& image);
    bool initializeSkybox(const std::string& filepath);
    Math::Vector3D skyGradient(const Math::Vector3D& unitDir);

} // namespace Skybox
