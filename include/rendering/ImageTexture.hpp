/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** ImageTexture
*/

#pragma once

#include "rendering/Texture.hpp"
#include "core/Vector3D.hpp"
#include <vector>
#include <string>
#include <istream>

namespace RayTracer {

    class ImageTexture : public Texture {
    public:
        explicit ImageTexture(const std::string &filename);

        Math::Vector3D sample(float u, float v) const override;

    private:
        static void skipComments(std::istream &is);

        std::vector<unsigned char> _data;
        int _width = 0;
        int _height = 0;
        int _channels = 0;
    };

}
