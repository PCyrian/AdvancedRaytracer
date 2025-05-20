/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** ShadowShading
*/

#pragma once

#include "Scene.hpp"
#include "Light.hpp"
#include "core/Ray.hpp"
#include "core/Vector3D.hpp"
#include "rendering/Material.hpp"
#include "primitives/Sphere.hpp"
#include "Skybox.hpp"

#include <limits>
#include <cmath>
#include <random>
#include <algorithm>
#include <initializer_list>

namespace RayTracer {
    double clamp01(double v);
    double rand01();
    Math::Vector3D sampleHemisphere(const Math::Vector3D &N_shading, double &pdf);
    Math::Vector3D shade(const Ray &ray, const Scene &scene, int depth, int maxDepth);
}