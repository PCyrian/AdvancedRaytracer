/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Material
*/

#pragma once

#include <string>
#include <memory>
#include "core/Vector3D.hpp"
#include "rendering/Texture.hpp"

namespace RayTracer {

  struct Material {
      std::string name;
      Math::Vector3D albedo;
      std::shared_ptr<Texture> diffuseMap;
      float roughness;
      float metallic;
      Math::Vector3D specular;
      float transparency;
      float ior;
      Math::Vector3D emission;
      float emissionStrength;

      Material();

      Material(const std::string& n,
               const Math::Vector3D& a,
               float r,
               float m,
               const Math::Vector3D& s,
               float t,
               float i);

      Material(const std::string& n,
               const Math::Vector3D& a,
               float r,
               float m,
               const Math::Vector3D& s,
               float t,
               float i,
               const Math::Vector3D& e);
  };

} // namespace RayTracer