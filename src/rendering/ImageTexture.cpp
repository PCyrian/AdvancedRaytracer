/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** ImageTexture
*/

#include "../../include/rendering/ImageTexture.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cmath>

RayTracer::ImageTexture::ImageTexture(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file)
        throw std::runtime_error("Cannot open texture file: " + filename);

    std::string magic;
    file >> magic;
    if (magic != "P6" && magic != "P3")
        throw std::runtime_error("Unsupported PPM format, must be P3 or P6: " + magic);

    skipComments(file);

    int w, h, maxv;
    file >> w >> h;
    skipComments(file);
    file >> maxv;
    file.get(); // consume newline

    if (w <= 0 || h <= 0 || maxv != 255)
        throw std::runtime_error("Invalid PPM dimensions or max value");

    _width = w;
    _height = h;
    _channels = 3;
    _data.clear();
    _data.reserve(w * h * 3);

    if (magic == "P6") {
        std::vector<unsigned char> buf(w * h * 3);
        file.read(reinterpret_cast<char*>(buf.data()), buf.size());
        if (!file)
            throw std::runtime_error("Error reading P6 pixel data");
        _data.insert(_data.end(), buf.begin(), buf.end());
    } else {
        for (int i = 0; i < w * h * 3; ++i) {
            int v;
            file >> v;
            if (!file)
                throw std::runtime_error("Error reading P3 pixel data");
            _data.push_back(static_cast<unsigned char>(v));
        }
    }
}

Math::Vector3D RayTracer::ImageTexture::sample(float u, float v) const {
    if (_width == 0 || _height == 0 || _channels < 3)
        return {1, 1, 1};

    u = u - std::floor(u);
    v = v - std::floor(v);

    int x = std::min(int(u * _width), _width - 1);
    int y = std::min(int(v * _height), _height - 1);
    int idx = (y * _width + x) * _channels;

    float r = _data[idx    ] / 255.0f;
    float g = _data[idx + 1] / 255.0f;
    float b = _data[idx + 2] / 255.0f;
    return { r, g, b };
}

void RayTracer::ImageTexture::skipComments(std::istream &is) {
    while (std::isspace(is.peek())) is.get();
    while (is.peek() == '#') {
        std::string line;
        std::getline(is, line);
        while (std::isspace(is.peek())) is.get();
    }
}