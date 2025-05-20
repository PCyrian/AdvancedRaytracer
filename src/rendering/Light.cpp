/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/Light.hpp"

RayTracer::Light RayTracer::Light::ambient(double i) {
    return { RayTracer::LightType::Ambient, {}, {}, i };
}

RayTracer::Light RayTracer::Light::directional(const Math::Vector3D& dir, double i) {
    return { RayTracer::LightType::Directional, dir, {}, i };
}

RayTracer::Light RayTracer::Light::point(const Math::Point3D& pos, double i) {
    return { RayTracer::LightType::Point, {}, pos, i };
}

