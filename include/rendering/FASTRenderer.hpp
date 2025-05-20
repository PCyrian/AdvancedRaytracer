/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** FASTRenderer
*/

#pragma once

#include <vector>


#include "../../include/core/Raytracer.hpp"
#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"

namespace Utils {

    Math::Vector3D FastShade(const RayTracer::Ray &ray, const RayTracer::Scene &scene);

    // Fast renderer: simple colors with ambient occlusion shadows
    std::vector<Math::Vector3D> FASTRenderer(const RayTracer::Scene &scene,
                                             const RenderParams &p,
                                             bool showProgress);
} // namespace Utils
