/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

/**
 * @file Light.hpp
 * @brief Defines light sources for the ray tracer.
 * @author Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05
 *
 * @details
 * The Light struct represents three types of lights:
 * - Ambient: Uniform illumination with no direction or position.
 * - Directional: Light with a specified direction, simulating distant light.
 * - Point: Light located at a specific position in space.
*/

#pragma once

#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"

namespace RayTracer {

    /**
     * @enum LightType
     * @brief Enumeration of supported light source types.
    */
    enum class LightType {
        Ambient,        ///< Uniform ambient light. 
        Directional,    ///< Directional light with a vector direction.
        Point           ///< Point light at a position in space.
    };

    /**
     * @struct Light
     * @brief Encapsulates a light source's properties.
     *
     * A Light can be one of three types: ambient, directional, or point.
     * Depending on type, only the relevant fields (direction or position)
     * are used.
    */
    struct Light {
        LightType type;             ///< Type of the light.
        Math::Vector3D direction;   ///< Direction vector for directional lights.
        Math::Point3D position;     ///< Position for point lights.
        double intensity;           ///< Brightness/intensity of the light.

        /**
         * @brief    Create an ambient light.
         * @param i  Intensity of the ambient light.
         * @return   Light instance configured as ambient.
        */
        static Light ambient(double i);

        /**
         * @brief      Create a directional light.
         * @param dir  Unit direction vector of the light rays.
         * @param i    Intensity of the directional light.
         * @return     Light instance configured as directional.
        */
        static Light directional(const Math::Vector3D& dir, double i);

        /**
         * @brief      Create a point light.
         * @param pos  World-space position of the point light.
         * @param i    Intensity of the point light.
         * @return     Light instance configured as point light.
        */
        static Light point(const Math::Point3D& pos, double i);
    };
}
