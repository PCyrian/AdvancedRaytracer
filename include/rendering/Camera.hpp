/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** Camera
*/

#pragma once

#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Ray.hpp"

#include <cmath>
#include <random>

namespace RayTracer {

    class Camera {
    public:
        Math::Point3D origin;
        Math::Point3D lower_left;
        Math::Vector3D horizontal, vertical;
        Math::Vector3D u, v, w;
        double lens_radius;
        int height, width;

        Camera(const Math::Point3D& lookfrom,
               const Math::Point3D& lookat,
               const Math::Vector3D& vup,
               double vfov,
               double aspect,
               double aperture = 0.0,
               double focus_dist = 1.0,
               int cam_height = 800,
               int cam_width = 800);

        Ray ray(double s, double t) const;

    private:
        static Math::Vector3D random_in_unit_disk();
    };

}