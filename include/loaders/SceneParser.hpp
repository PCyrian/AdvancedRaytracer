/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#pragma once

#include <string>

#include "rendering/Scene.hpp"

namespace RayTracer {
    bool loadScene(const std::string &filename, Scene &outScene, bool debug);
}
