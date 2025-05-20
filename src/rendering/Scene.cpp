/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/Scene.hpp"

#include "../../include/rendering/Scene.hpp"

RayTracer::Scene::Scene(const RayTracer::Camera& cam)
    : camera(cam) {
}

void RayTracer::Scene::addObject(const std::shared_ptr<RayTracer::Primitive>& obj) {
    objects.push_back(obj);
}

void RayTracer::Scene::addLight(const RayTracer::Light& light) {
    lights.push_back(light);
}

bool RayTracer::Scene::hit(const RayTracer::Ray& ray, double& tOut, RayTracer::HitRecord& rec) const {
    return bvhRoot->hit(ray, tOut, rec);
}
