/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Scene
*/

#pragma once

#include <vector>
#include <memory>

#include "core/Primitive.hpp"
#include "Camera.hpp"
#include "Light.hpp"
#include "BVH.hpp"

namespace RayTracer {
    class Scene {
    public:
        Camera camera;
        std::vector<std::shared_ptr<Primitive>> objects;
        std::unique_ptr<BVHNode> bvhRoot;
        std::vector<Light> lights;

        explicit Scene(const Camera& cam);

        void addObject(const std::shared_ptr<Primitive>& obj);
        void addLight(const Light& light);
        bool hit(const Ray& ray, double& tOut, HitRecord& rec) const;
    };
}
