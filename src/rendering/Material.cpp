/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/rendering/Material.hpp"

RayTracer::Material::Material()
    : name{},
      albedo{0,0,0},
      diffuseMap{nullptr},
      roughness{0},
      metallic{0},
      specular{0,0,0},
      transparency{0},
      ior{1},
      emission{0,0,0},
      emissionStrength{1.0f}
{}

RayTracer::Material::Material(const std::string& n,
    const Math::Vector3D& a,
    float r,
    float m,
    const Math::Vector3D& s,
    float t,
    float i)
    : name(n),
      albedo(a),
      diffuseMap(nullptr),
      roughness(r),
      metallic(m),
      specular(s),
      transparency(t),
      ior(i),
      emission{0,0,0},
      emissionStrength{1.0f}
{}

RayTracer::Material::Material(const std::string& n,
    const Math::Vector3D& a,
    float r,
    float m,
    const Math::Vector3D& s,
    float t,
    float i,
    const Math::Vector3D& e)
    : name(n),
      albedo(a),
      diffuseMap(nullptr),
      roughness(r),
      metallic(m),
      specular(s),
      transparency(t),
      ior(i),
      emission(e),
      emissionStrength{1.0f}
{}
