/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/core/Primitive.hpp"

const RayTracer::Material& RayTracer::Primitive::getMaterial() const {
    return material;
}
