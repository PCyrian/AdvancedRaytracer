/*
** EPITECH PROJECT, 2025
** B-OOP-400-STG-4-1-raytracer-noe.carabin
** File description:
** BDPT
*/

/**
 * @file BDPT.hpp
 * @brief Implements Bidirectional Path Tracing (BDPT) for better caustics
 * @authors Cyrian Pittaluga
 * @version 1.0
 * @date 2025-05-14
 *
 * @details
 * This file contains the core logic for the BDPT algorithm, including:
 * - Path generation for camera and light sub-paths.
 * - BSDF evaluation and sampling for various material types.
 * - Geometric term calculation for connecting sub-paths.
 * - Multiple Importance Sampling (MIS) for weighting path contributions.
 * - The main rendering loop that integrates these components.
 *
 */

#ifndef BDPT_HPP
#define BDPT_HPP

#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <memory>
#include <limits>
#include <iostream>

#include "core/Point3D.hpp"
#include "core/Vector3D.hpp"
#include "core/Vector2D.hpp"
#include "core/Ray.hpp"
#include "core/Primitive.hpp"
#include "core/Vertex.hpp"
#include "core/Utils.hpp"

#include "rendering/Material.hpp"
#include "rendering/Texture.hpp"
#include "rendering/Light.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Scene.hpp"
#include "rendering/Skybox.hpp"

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

/**
 * @namespace BDPT
 * @brief Contains constants, utility functions, and the main rendering logic for Bidirectional Path Tracing.
 */
namespace BDPT {
    const int RR_START_DEPTH = 3;                ///< Depth at which Russian Roulette path termination begins.
    const double PDF_EPSILON = 1e-7;             ///< Small epsilon value for Probability Density Function (PDF) comparisons to avoid division by zero or instability.
    const double GEOMETRY_EPSILON = 1e-8;        ///< Small epsilon value for geometric comparisons (e.g., vector lengths, distances) to handle floating-point inaccuracies.
    const double THROUGHPUT_EPSILON_SQ = 1e-12;  ///< Squared epsilon for path throughput magnitude checks; paths with throughput below this are considered negligible.
    const double RAY_OFFSET_EPSILON = 1e-4;      ///< Small distance to offset rays from surfaces to prevent self-intersection.
    const double ONE_OVER_PI = 1.0 / M_PI;       ///< Precomputed value of 1/PI.
    const double ONE_OVER_FOUR_PI = 1.0 / (4.0 * M_PI); ///< Precomputed value of 1/(4*PI).

} // namespace BDPT

namespace BDPT {

    /**
     * @brief Generates a pseudo-random double precision number in the range [0, 1).
     * @details This function uses a thread-local random number generator to ensure thread safety
     * and distinct sequences for parallel execution contexts (e.g., OpenMP).
     * @return A random double between 0.0 (inclusive) and 1.0 (exclusive).
     */
    inline double local_rand01() {
        thread_local static std::mt19937 gen(std::random_device{}() +
                                             #ifdef _OPENMP
                                             omp_get_thread_num()
                                             #else
                                             0
                                             #endif
        );
        thread_local static std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    /**
     * @brief Normalizes a 3D vector safely.
     * @details If the vector's length is below a small epsilon (GEOMETRY_EPSILON),
     * a zero vector is returned to prevent division by zero or instability.
     * @param v The vector to normalize.
     * @return The normalized vector, or a zero vector if the original length is negligible.
     */
    inline Math::Vector3D safe_normalize(const Math::Vector3D& v) {
        double l = v.length();
        if (l < GEOMETRY_EPSILON) return Math::Vector3D(0,0,0);
        return v / l;
    }

    /**
     * @brief Calculates the reflection vector for a given incident vector and surface normal.
     * @param v_incident_towards_surface The incident vector, pointing towards the surface.
     * @param normal_n The surface normal, assumed to be unit length and pointing outwards from the surface.
     * @return The reflection vector.
     */
    inline Math::Vector3D reflect_vec(const Math::Vector3D& v_incident_towards_surface, const Math::Vector3D& normal_n) {
        return v_incident_towards_surface - normal_n * 2.0 * v_incident_towards_surface.dot(normal_n);
    }

    /**
     * @brief Calculates the refraction vector using Snell's Law.
     * @param incident_I_pointing_towards_surface The incident vector, pointing towards the surface.
     * @param N_interface_from_inc_to_trans The interface normal, oriented from the incident medium towards the transmitted medium. Assumed unit length.
     * @param eta_ratio_incident_over_transmitted The ratio of refractive indices (n_incident / n_transmitted).
     * @return The refraction vector. Returns a zero vector if Total Internal Reflection (TIR) occurs.
     */
    inline Math::Vector3D refract_vec(const Math::Vector3D& incident_I_pointing_towards_surface, const Math::Vector3D& N_interface_from_inc_to_trans, double eta_ratio_incident_over_transmitted) {
        double cos_theta_i = -incident_I_pointing_towards_surface.dot(N_interface_from_inc_to_trans);
        cos_theta_i = std::max(-1.0, std::min(1.0, cos_theta_i)); // Clamp

        double sin2_theta_t = eta_ratio_incident_over_transmitted * eta_ratio_incident_over_transmitted * (1.0 - cos_theta_i * cos_theta_i);

        if (sin2_theta_t > 1.000001) return Math::Vector3D(0,0,0);

        double cos_theta_t = std::sqrt(std::max(0.0, 1.0 - sin2_theta_t));
        return incident_I_pointing_towards_surface * eta_ratio_incident_over_transmitted +
               N_interface_from_inc_to_trans * (eta_ratio_incident_over_transmitted * cos_theta_i - cos_theta_t);
    }

    /**
     * @brief Calculates the Fresnel reflection coefficient using Schlick's approximation.
     * @param cos_incident_angle_wrt_normal Cosine of the angle between the incident ray and the surface normal. Should be positive.
     * @param n1_incident_medium Index of Refraction (IOR) of the incident medium.
     * @param n2_transmitted_medium Index of Refraction (IOR) of the transmitted medium.
     * @return The scalar Fresnel reflection coefficient (reflectance). Returns 1.0 for Total Internal Reflection (TIR).
     */
    inline double fresnel_schlick_scalar(double cos_incident_angle_wrt_normal, double n1_incident_medium, double n2_transmitted_medium) {
        if (n1_incident_medium == n2_transmitted_medium) return 0.0;

        double R0_num = (n1_incident_medium - n2_transmitted_medium);
        double R0_den = (n1_incident_medium + n2_transmitted_medium);
        if (std::abs(R0_den) < PDF_EPSILON) return 1.0;
        double R0 = R0_num / R0_den;
        R0 = R0 * R0;

        // Explicitly check for Total Internal Reflection (TIR) if ray is coming from a denser medium.
        if (n1_incident_medium > n2_transmitted_medium) {
            double sin2_theta_i = 1.0 - cos_incident_angle_wrt_normal * cos_incident_angle_wrt_normal;
            if (sin2_theta_i < 0.0) sin2_theta_i = 0.0; // Clamp
            double n_ratio_sq = (n1_incident_medium / n2_transmitted_medium) * (n1_incident_medium / n2_transmitted_medium);
            double sin2_theta_t_check = n_ratio_sq * sin2_theta_i;
            if (sin2_theta_t_check > 1.0) return 1.0; // Total Internal Reflection.
        }

        double term = 1.0 - std::max(0.0, cos_incident_angle_wrt_normal);
        return R0 + (1.0 - R0) * term * term * term * term * term; // Schlick's approximation.
    }

    /**
     * @brief Samples a direction on a cosine-weighted hemisphere defined by a normal vector.
     * @param normal The normal vector defining the hemisphere's orientation (local Z-axis).
     * @param[out] pdf_out The Probability Density Function (PDF) of sampling the returned direction, in solid angle measure (cos(theta)/PI).
     * @return A randomly sampled direction vector on the hemisphere.
     */
    inline Math::Vector3D sample_cosine_hemisphere_bdpt(const Math::Vector3D& normal, double& pdf_out) {
        double r1 = local_rand01();
        double r2 = local_rand01();

        double sin_theta_sample = std::sqrt(r1);
        double cos_theta_sample = std::sqrt(1.0 - r1);
        double phi = 2.0 * M_PI * r2;

        Math::Vector3D w = normal;
        Math::Vector3D t_candidate = (std::abs(w.x) > 0.1 ? Math::Vector3D(0, 1, 0) : Math::Vector3D(1, 0, 0));
        Math::Vector3D u_basis = safe_normalize(t_candidate.cross(w));
        Math::Vector3D v_basis = w.cross(u_basis);

        pdf_out = cos_theta_sample * BDPT::ONE_OVER_PI;
        if (pdf_out < PDF_EPSILON) pdf_out = PDF_EPSILON;

        return safe_normalize(u_basis * std::cos(phi) * sin_theta_sample +
                              v_basis * std::sin(phi) * sin_theta_sample +
                              w * cos_theta_sample);
    }

    /**
     * @brief Samples a direction uniformly over the unit sphere.
     * @param[out] pdf_out The Probability Density Function (PDF) of sampling the returned direction, in solid angle measure (1/(4*PI)).
     * @return A randomly sampled unit direction vector.
     */
    inline Math::Vector3D sample_uniform_sphere_direction_bdpt(double& pdf_out) {
        double z = 1.0 - 2.0 * local_rand01();
        double r_xy = std::sqrt(std::max(0.0, 1.0 - z*z));
        double phi = 2.0 * M_PI * local_rand01();
        pdf_out = BDPT::ONE_OVER_FOUR_PI;
        return Math::Vector3D(r_xy * std::cos(phi), r_xy * std::sin(phi), z);
    }

    /**
     * @typedef Path
     * @brief Represents a sequence of vertices forming a light path (either camera or light sub-path).
     * @details Each vertex in the path stores geometric information, material properties, BSDF throughput, and PDFs.
     */
    using Path = std::vector<RayTracer::Vertex>;

    /**
     * @brief Retrieves the effective albedo of a material at a hit point.
     * @details Considers the material's base albedo and any diffuse texture map applied.
     * If a diffuse texture exists, it samples the texture at the given UV coordinates.
     * @param mat The material of the hit surface.
     * @param rec The hit record containing UV coordinates and other surface information.
     * @return The albedo color (RGB vector) of the material at the hit point.
     */
    inline Math::Vector3D get_material_albedo_from_rec(const RayTracer::Material& mat, const RayTracer::HitRecord& rec) {
        if (mat.diffuseMap) {
            return mat.diffuseMap->sample(rec.uv.x, rec.uv.y);
        }
        return mat.albedo;
    }

    /**
     * @brief Evaluates the Bidirectional Scattering Distribution Function (BSDF) for a pair of directions.
     * @details This function is primarily used for path connection strategies in BDPT. It provides an approximate
     * BSDF value for non-delta scattering events. For delta BSDFs (perfect mirrors/glass), this function
     * may return zero as their contribution is handled by specific sampling.
     * @param rec The hit record at the scattering surface.
     * @param wo_world The outgoing direction (from surface to observer/next vertex) in world space.
     * @param wi_world The incoming direction (from light/previous vertex to surface) in world space.
     * @param N_geom The geometric normal of the surface (unused in this simplified version, but common in full BSDFs).
     * @param wi_is_transmission_hint A hint if the incoming direction might be due to transmission (unused here).
     * @return The BSDF value (RGB vector) for the given directions. Returns (0,0,0) if the interaction is delta or non-contributing for connections.
     */
    Math::Vector3D eval_bsdf(const RayTracer::HitRecord& rec, const Math::Vector3D& wo_world, const Math::Vector3D& wi_world,
                             [[maybe_unused]] const Math::Vector3D& N_geom, [[maybe_unused]] bool wi_is_transmission_hint) {
        const RayTracer::Material& M = rec.material;
        Math::Vector3D albedo = get_material_albedo_from_rec(M, rec);

        Math::Vector3D N_eff = rec.normal;
        if (N_eff.dot(wo_world) < 0.0) N_eff = N_eff * -1.0;

        double N_eff_dot_wi = N_eff.dot(wi_world);

        // --- Opaque Non-Metallic (Diffuse/Glossy Dielectric) ---
        if (M.metallic < 0.5 && M.transparency < 0.5) {
            if (N_eff_dot_wi > 0.0) {
                return albedo * BDPT::ONE_OVER_PI;
            }
        }
        // --- Metallic ---
        else if (M.metallic >= 0.5) {
            if (M.roughness <= 0.05f) return Math::Vector3D(0,0,0); // Perfect mirrors are delta events, eval is 0 for arbitrary wi.
            if (N_eff_dot_wi > 0.0) {
                return M.specular * BDPT::ONE_OVER_PI;
            }
        }
        // --- Transparent Dielectric ---
        else if (M.transparency >= 0.5) {
            if (M.roughness <= 0.05f) return Math::Vector3D(0,0,0); // Perfect dielectrics

            // Approximate BSDF for rough dielectrics
            bool wo_is_exiting_material = rec.normal.dot(wo_world) > 0.0;
            double n1 = wo_is_exiting_material ? M.ior : 1.0;
            double n2 = wo_is_exiting_material ? 1.0 : M.ior;

            double cos_theta_o_abs = std::abs(N_eff.dot(wo_world)); // Cosine of outgoing angle w.r.t effective normal.
            cos_theta_o_abs = std::min(1.0, std::max(0.0, cos_theta_o_abs)); // Clamp

            double R_fresnel = fresnel_schlick_scalar(cos_theta_o_abs, n1, n2);

            if (N_eff_dot_wi * N_eff.dot(wo_world) > 0) {
                return M.specular * R_fresnel * BDPT::ONE_OVER_PI;
            } else { // Transmission.
                Math::Vector3D transmission_tint = albedo;
                return transmission_tint * (1.0 - R_fresnel) * BDPT::ONE_OVER_PI;
            }
        }
        return Math::Vector3D(0,0,0);
    }

    /**
     * @brief Samples an incoming light direction (wi_sampled_world) based on material properties and an outgoing view direction (wo_world).
     * @details This function implements importance sampling for various material types (metallic, dielectric, diffuse).
     * It determines the next path direction, the BSDF value for that direction, the PDF of sampling it,
     * and whether the scattering event was a delta distribution (e.g., perfect mirror).
     * @param rec The hit record at the scattering surface.
     * @param wo_world The outgoing direction from the surface (e.g., towards camera or previous vertex on light path), in world space.
     * @param[out] wi_sampled_world The sampled incoming light direction (or next path direction) in world space.
     * @param[out] pdf_solid_angle_wi The Probability Density Function (PDF) of sampling wi_sampled_world, in solid angle measure.
     * @param[out] bsdf_term_value The BSDF throughput value (f_s * cos(theta_i) / pdf_wi if not absorbing pdf here) for the sampled direction. Typically (f_s * albedo_or_specular_tint).
     * @param[out] scattered_is_delta True if the scattering event is a delta distribution (e.g., perfect mirror/glass), false otherwise.
     */
    void sample_bsdf(const RayTracer::HitRecord& rec, const Math::Vector3D& wo_world,
                     Math::Vector3D& wi_sampled_world, double& pdf_solid_angle_wi,
                     Math::Vector3D& bsdf_term_value, bool& scattered_is_delta) {

        const RayTracer::Material& M = rec.material;
        Math::Vector3D albedo_val = get_material_albedo_from_rec(M, rec);

        scattered_is_delta = false;
        bsdf_term_value = Math::Vector3D(0,0,0);
        pdf_solid_angle_wi = 0.0;
        wi_sampled_world = Math::Vector3D(0,0,0);

        Math::Vector3D V_incident_path = wo_world * -1.0;

        bool is_entering_object_boundary = V_incident_path.dot(rec.normal) < 0.0;

        Math::Vector3D N_shading_oriented;
        Math::Vector3D N_interface_for_snell;
        double n_incident_medium_ior;
        double n_transmitted_medium_ior;

        double base_dielectric_ior_for_highlights = 1.5;

        if (is_entering_object_boundary) {
            N_shading_oriented = rec.normal;
            N_interface_for_snell = rec.normal;
            n_incident_medium_ior = 1.0;
            n_transmitted_medium_ior = M.ior;
        } else {
            N_shading_oriented = rec.normal * -1.0;
            N_interface_for_snell = rec.normal * -1.0;
            n_incident_medium_ior = M.ior;
            n_transmitted_medium_ior = 1.0;
        }

        double N_dot_V = std::min(1.0, std::max(0.0, -V_incident_path.dot(N_shading_oriented)));
        Math::Vector3D material_specular_f0_tint = M.specular;


        if (M.metallic >= 0.5f) { // --- METALLIC MATERIAL ---
            scattered_is_delta = (M.roughness < 0.05f); // Smooth metals are perfect mirrors (delta).
            wi_sampled_world = reflect_vec(V_incident_path, N_shading_oriented); // Metals only reflect.
            bsdf_term_value = material_specular_f0_tint;

            if (scattered_is_delta) {
                pdf_solid_angle_wi = 1.0;
            } else {
                sample_cosine_hemisphere_bdpt(N_shading_oriented, pdf_solid_angle_wi);
            }

        } else if (M.transparency >= 0.5f) { // --- TRANSPARENT DIELECTRIC (e.g., Glass, Water) ---
            scattered_is_delta = (M.roughness < 0.05f);

            double fresnel_reflect_prob = fresnel_schlick_scalar(N_dot_V, n_incident_medium_ior, n_transmitted_medium_ior);

            if (local_rand01() < fresnel_reflect_prob) {
                wi_sampled_world = reflect_vec(V_incident_path, N_shading_oriented);
                bsdf_term_value = material_specular_f0_tint;
            } else {
                double eta_ratio = n_incident_medium_ior / n_transmitted_medium_ior;
                wi_sampled_world = refract_vec(V_incident_path, N_interface_for_snell, eta_ratio);

                if (wi_sampled_world.dot(wi_sampled_world) < GEOMETRY_EPSILON * GEOMETRY_EPSILON) {
                    wi_sampled_world = reflect_vec(V_incident_path, N_shading_oriented);
                    bsdf_term_value = material_specular_f0_tint;
                } else { // Successful transmission.
                    bsdf_term_value = albedo_val;
                }
            }
            // PDF for delta reflection/transmission.
            if (scattered_is_delta) {
                pdf_solid_angle_wi = 1.0;
            } else {
                // For rough dielectrics, ideally use microfacet model for sampling.
                sample_cosine_hemisphere_bdpt(N_shading_oriented, pdf_solid_angle_wi);
            }

        } else { // --- OPAQUE NON-METALLIC (e.g., Diffuse, Plastic, Matte) ---
            double physical_fresnel_prob_for_highlights = fresnel_schlick_scalar(N_dot_V, 1.0, base_dielectric_ior_for_highlights);

            if (material_specular_f0_tint.dot(material_specular_f0_tint) < 0.001f && M.roughness > 0.5f) {
                physical_fresnel_prob_for_highlights = 0.0; // Force diffuse path.
            }

            if (local_rand01() < physical_fresnel_prob_for_highlights) { // Sample Specular Lobe (for highlights).
                scattered_is_delta = (M.roughness < 0.05f); // Glossy highlights are non-delta if roughness > ~0.05.
                wi_sampled_world = reflect_vec(V_incident_path, N_shading_oriented);
                bsdf_term_value = material_specular_f0_tint;

                if (scattered_is_delta) {
                    pdf_solid_angle_wi = 1.0;
                } else {
                    sample_cosine_hemisphere_bdpt(N_shading_oriented, pdf_solid_angle_wi);
                }
            } else { // Sample Diffuse Lobe.
                scattered_is_delta = false;
                wi_sampled_world = sample_cosine_hemisphere_bdpt(N_shading_oriented, pdf_solid_angle_wi);
                if (pdf_solid_angle_wi > PDF_EPSILON) {
                    bsdf_term_value = albedo_val;
                } else {
                    bsdf_term_value = Math::Vector3D(0,0,0);
                    wi_sampled_world = Math::Vector3D(0,0,0);
                }
            }
        }

        if (wi_sampled_world.dot(wi_sampled_world) < GEOMETRY_EPSILON * GEOMETRY_EPSILON ) {
            pdf_solid_angle_wi = 0.0;
            bsdf_term_value = Math::Vector3D(0,0,0);
            scattered_is_delta = true;
        } else if (pdf_solid_angle_wi < PDF_EPSILON && !scattered_is_delta) {
            bsdf_term_value = Math::Vector3D(0,0,0);
        }
        if (!scattered_is_delta && pdf_solid_angle_wi < PDF_EPSILON && bsdf_term_value.dot(bsdf_term_value) > GEOMETRY_EPSILON) {
             pdf_solid_angle_wi = PDF_EPSILON;
        }
    }

    /**
     * @brief Generates a sub-path (either camera or light) by iteratively sampling BSDFs and tracing rays.
     * @details The path starts from `current_ray` and extends up to `max_bounces`.
     * Vertices are stored in `path_vertices`. Russian Roulette is used for path termination.
     * @param scene The scene containing geometry and materials.
     * @param current_ray The initial ray for starting the sub-path.
     * @param max_bounces The maximum number of bounces (path segments) allowed for this sub-path.
     * @param[out] path_vertices The generated list of vertices forming the sub-path.
     * @param is_camera_path True if generating a camera sub-path, false for a light sub-path.
     * @param params Global rendering parameters (unused in this specific simplified version but often needed).
     * @return The number of vertices generated in the `path_vertices` list.
     */
    int generate_subpath_fixed(const RayTracer::Scene& scene, RayTracer::Ray current_ray, int max_bounces,
                               Path& path_vertices, bool is_camera_path, [[maybe_unused]] const Utils::RenderParams& params) {
        path_vertices.clear();
        Math::Vector3D beta(1.0, 1.0, 1.0);
        double pdf_fwd_prev_bounce_solid_angle = 1.0;

        for (int bounce = 0; bounce <= max_bounces; ++bounce) {
            RayTracer::HitRecord rec_hit;
            double t_hit = std::numeric_limits<double>::max();
            bool hit_surface = scene.hit(current_ray, t_hit, rec_hit);

            RayTracer::Vertex v;
            v.type = RayTracer::VertexType::Surface;
            v.wi = current_ray.direction * -1.0;
            v.beta = beta;
            v.pdfFwd = pdf_fwd_prev_bounce_solid_angle;
            v.is_point_light_source = false;

            if (!hit_surface) {
                if (is_camera_path) {
                    RayTracer::Vertex sky_v;
                    sky_v.type = RayTracer::VertexType::Surface;
                    sky_v.rec.t = std::numeric_limits<double>::infinity();
                    sky_v.rec.point = current_ray.origin + current_ray.direction * 1e10;
                    sky_v.rec.normal = current_ray.direction * -1.0;
                    sky_v.rec.material.emission = Skybox::skyGradient(current_ray.direction);
                    sky_v.rec.material.emissionStrength = 1.0f;
                    sky_v.rec.material.albedo = Math::Vector3D(0,0,0);
                    sky_v.wi = current_ray.direction * -1.0f;
                    sky_v.beta = beta;
                    sky_v.pdfFwd = pdf_fwd_prev_bounce_solid_angle;
                    sky_v.delta = false;
                    sky_v.is_point_light_source = false;
                    path_vertices.push_back(sky_v);
                }
                break; // End of path if ray misses
            }

            v.rec = rec_hit;

            // Determine if the BSDF at this vertex is a delta function (perfect specular).
            bool bsdf_at_this_vertex_is_delta = false;
            const auto& M_curr = rec_hit.material;
            if (M_curr.metallic >= 0.5f && M_curr.roughness < 0.05f) { // Smooth metal.
                bsdf_at_this_vertex_is_delta = true;
            } else if (M_curr.transparency >= 0.5f && M_curr.roughness < 0.05f) { // Smooth dielectric.
                bsdf_at_this_vertex_is_delta = true;
            }
            v.delta = bsdf_at_this_vertex_is_delta;

            path_vertices.push_back(v);

            if (v.rec.material.emission.dot(v.rec.material.emission) * v.rec.material.emissionStrength > GEOMETRY_EPSILON) {
                 if (!is_camera_path && bounce == 0) {
                 } else {
                     break;
                 }
            }

            // Sample next direction from BSDF at the current vertex.
            Math::Vector3D wi_sampled_next_direction;    // Sampled direction for the next ray.
            double pdf_for_wi_sampled_next_direction;  // PDF of sampling that direction.
            Math::Vector3D bsdf_throughput_factor;     // BSDF value (f_s * albedo/specular) for the sampled direction.
            bool next_ray_will_be_from_delta_scatter; // Was the BSDF sample a delta event?

            sample_bsdf(rec_hit, current_ray.direction * -1.0,
                        wi_sampled_next_direction, pdf_for_wi_sampled_next_direction,
                        bsdf_throughput_factor, next_ray_will_be_from_delta_scatter);

            if ( (pdf_for_wi_sampled_next_direction < PDF_EPSILON && !next_ray_will_be_from_delta_scatter) ||
                 bsdf_throughput_factor.dot(bsdf_throughput_factor) < THROUGHPUT_EPSILON_SQ ) {
                break;
            }

            beta = beta * bsdf_throughput_factor;

            if (!path_vertices.empty()) {
                path_vertices.back().pdfRev = pdf_for_wi_sampled_next_direction;
            }

            // Update PDF for the next bounce.
            pdf_fwd_prev_bounce_solid_angle = pdf_for_wi_sampled_next_direction;

            // Create next ray.
            current_ray = RayTracer::Ray(rec_hit.point + wi_sampled_next_direction * RAY_OFFSET_EPSILON,
                                         wi_sampled_next_direction);

            // Russian Roulette for path termination.
            if (bounce >= BDPT::RR_START_DEPTH) {
                double max_beta_comp = std::max({beta.x, beta.y, beta.z});
                if (max_beta_comp < 0.0) max_beta_comp = 0.0;
                double p_continue = std::min(0.95, max_beta_comp);

                if (local_rand01() > p_continue) { break; }
                if (p_continue < PDF_EPSILON) break;
                beta = beta / p_continue;
            }

            if (beta.dot(beta) < THROUGHPUT_EPSILON_SQ) break;
        }
        return path_vertices.size();
    }

    /**
     * @brief Samples a light source from the scene to initiate a light sub-path.
     * @details Selects a light source (either from explicit lights list or emissive scene objects)
     * and samples a point on it and an initial emission direction.
     * @param scene The scene containing light sources and emissive objects.
     * @param[out] light_ray_out The initial ray emitted from the sampled light source.
     * @param[out] Le_value_out The radiance (L_e) of the sampled light source at the point of emission.
     * @param[out] pdf_pos_area_light_out The PDF of sampling the position on the light source (area measure for area lights, discrete prob for point lights).
     * @param[out] pdf_emit_dir_solid_angle_out The PDF of sampling the initial emission direction (solid angle measure).
     * @param[out] light_vertex_out The first vertex (z_0) of the light sub-path, representing the point on the light source.
     * @return True if a light source was successfully sampled, false otherwise.
     */
    bool sample_light_source_fixed(const RayTracer::Scene& scene, RayTracer::Ray& light_ray_out, Math::Vector3D& Le_value_out,
                                   double& pdf_pos_area_light_out, double& pdf_emit_dir_solid_angle_out,
                                   RayTracer::Vertex& light_vertex_out) {
        light_vertex_out.is_point_light_source = false;

        std::vector<const RayTracer::Primitive*> emissive_prims;
        for(const auto& obj_sptr : scene.objects) {
            const auto& mat = obj_sptr->getMaterial();
            if ((mat.emission.dot(mat.emission) > GEOMETRY_EPSILON) && mat.emissionStrength > GEOMETRY_EPSILON) {
                emissive_prims.push_back(obj_sptr.get());
            }
        }

        bool has_explicit_lights = !scene.lights.empty();
        bool has_emissive_prims = !emissive_prims.empty();

        if (!has_explicit_lights && !has_emissive_prims) return false;

        double prob_pick_explicit_category = 0.0;
        if(has_explicit_lights && has_emissive_prims) prob_pick_explicit_category = 0.5;
        else if(has_explicit_lights) prob_pick_explicit_category = 1.0;

        bool sample_from_explicit_list = local_rand01() < prob_pick_explicit_category;

        if (sample_from_explicit_list && has_explicit_lights) { // --- Sample from explicit scene.lights list ---
            int light_idx = static_cast<int>(local_rand01() * scene.lights.size());
            light_idx = std::min(light_idx, (int)scene.lights.size() - 1);
            const RayTracer::Light& L_obj = scene.lights[light_idx];

            if (L_obj.type == RayTracer::LightType::Point) {
                Le_value_out = Math::Vector3D(L_obj.intensity, L_obj.intensity, L_obj.intensity);
                light_ray_out.origin = L_obj.position;
                light_ray_out.direction = sample_uniform_sphere_direction_bdpt(pdf_emit_dir_solid_angle_out);

                double p_sample_this_light_from_explicits = 1.0 / static_cast<double>(scene.lights.size());
                pdf_pos_area_light_out = prob_pick_explicit_category * p_sample_this_light_from_explicits;

                light_vertex_out.rec.point = L_obj.position;
                light_vertex_out.rec.material.emission = Le_value_out;
                light_vertex_out.rec.material.emissionStrength = 1.0;
                light_vertex_out.rec.normal = Math::Vector3D(0,0,1);
                light_vertex_out.is_point_light_source = true;
                light_vertex_out.delta = true;
                return true;
            }
        } else if (has_emissive_prims) { // --- Sample from emissive scene objects (area lights) ---
            int emissive_obj_idx = static_cast<int>(local_rand01() * emissive_prims.size());
            emissive_obj_idx = std::min(emissive_obj_idx, (int)emissive_prims.size() - 1);
            const RayTracer::Primitive* emissive_prim = emissive_prims[emissive_obj_idx];

            if (const RayTracer::Sphere* sph_light = dynamic_cast<const RayTracer::Sphere*>(emissive_prim)) {
                double sphere_surface_area = 4.0 * M_PI * sph_light->radius * sph_light->radius;
                if (sphere_surface_area < GEOMETRY_EPSILON) return false;

                double p_sample_emissive_category = (has_explicit_lights && has_emissive_prims) ? (1.0 - prob_pick_explicit_category) : 1.0;
                double p_this_prim_from_emissives = 1.0 / static_cast<double>(emissive_prims.size());
                double p_point_on_surface_area = 1.0 / sphere_surface_area;
                pdf_pos_area_light_out = p_sample_emissive_category * p_this_prim_from_emissives * p_point_on_surface_area;

                double pdf_dummy_dir_sphere;
                Math::Vector3D dir_from_center = sample_uniform_sphere_direction_bdpt(pdf_dummy_dir_sphere);
                light_vertex_out.rec.point = sph_light->center + dir_from_center * sph_light->radius;
                light_vertex_out.rec.normal = dir_from_center;
                light_vertex_out.rec.material = sph_light->getMaterial();

                Le_value_out = light_vertex_out.rec.material.emission * light_vertex_out.rec.material.emissionStrength;

                light_ray_out.origin = light_vertex_out.rec.point + light_vertex_out.rec.normal * RAY_OFFSET_EPSILON;
                light_ray_out.direction = sample_cosine_hemisphere_bdpt(light_vertex_out.rec.normal, pdf_emit_dir_solid_angle_out);

                light_vertex_out.is_point_light_source = false;
                light_vertex_out.delta = false;
                return true;
            }
            // TODO: Add handling for other emissive primitive types (e.g., emissive quads, triangles from meshes).
        }
        return false;
    }


    /**
     * @brief Calculates the geometric term G(v0, v1) for connecting two vertices in BDPT.
     * @details The geometric term accounts for the inverse square distance between vertices,
     * cosine factors at both vertices (if they are non-delta surfaces), and visibility (occlusion).
     * @param scene The scene used for visibility checks (tracing shadow rays).
     * @param v0 The first vertex of the connection segment.
     * @param v1 The second vertex of the connection segment.
     * @return The geometric term G. Returns 0.0 if vertices are occluded or coincident.
     */
    double geometry_term_fixed(const RayTracer::Scene& scene, const RayTracer::Vertex& v0, const RayTracer::Vertex& v1) {
        Math::Vector3D d_vec = v1.rec.point - v0.rec.point;
        double dist_sq = d_vec.dot(d_vec);

        if (dist_sq < GEOMETRY_EPSILON * GEOMETRY_EPSILON) return 0.0;

        double dist = std::sqrt(dist_sq);
        Math::Vector3D dir_v0_to_v1 = d_vec / dist; // Normalized direction from v0 to v1.

        // --- Visibility Check (Shadow Ray) ---
        Math::Point3D shadow_ray_origin_pt = v0.rec.point;
        Math::Vector3D offset_normal_v0 = v0.rec.normal;
        if (!v0.is_point_light_source) {
            if (offset_normal_v0.dot(dir_v0_to_v1) < 0.0) offset_normal_v0 *= -1.0;
            shadow_ray_origin_pt = shadow_ray_origin_pt + (offset_normal_v0 * RAY_OFFSET_EPSILON);
        } else {
            shadow_ray_origin_pt = shadow_ray_origin_pt + (dir_v0_to_v1 * RAY_OFFSET_EPSILON);
        }

        RayTracer::Ray shadow_ray(shadow_ray_origin_pt, dir_v0_to_v1);
        RayTracer::HitRecord shadow_rec;
        double t_max_shadow = dist - RAY_OFFSET_EPSILON * 2.0;

        if (t_max_shadow > GEOMETRY_EPSILON) {
            if (scene.hit(shadow_ray, t_max_shadow, shadow_rec)) {
                return 0.0;
            }
        } else if (dist > RAY_OFFSET_EPSILON * 3.0) {
            return 0.0;
        }
        // --- Cosine Factors ---
        double cos0 = 1.0;
        if (!v0.is_point_light_source && !v0.delta) {
            Math::Vector3D N0_shading = v0.rec.normal;
            cos0 = std::abs(N0_shading.dot(dir_v0_to_v1));
        }

        double cos1 = 1.0;
        if (!v1.is_point_light_source && !v1.delta) {
            if(v1.rec.t != std::numeric_limits<double>::infinity()){
                Math::Vector3D N1_shading = v1.rec.normal;
                cos1 = std::abs(N1_shading.dot(dir_v0_to_v1 * -1.0));
            }
        }
        return (cos0 * cos1) / dist_sq;
    }


    /**
     * @brief Calculates the Multiple Importance Sampling (MIS) weight using the balance heuristic.
     * @details Weight = pdf_camera / (pdf_camera + pdf_light).
     * @param p_cam The PDF of generating the combined path using the camera sub-path strategy.
     * @param p_light The PDF of generating the combined path using the light sub-path strategy.
     * @return The MIS weight.
     */
    inline double mis_balance(double p_cam, double p_light) {
        double sum_pdfs = p_cam + p_light;
        if (sum_pdfs < PDF_EPSILON) return 0.0;
        return p_cam / sum_pdfs;
    }

    /**
     * @brief Calculates the Multiple Importance Sampling (MIS) weight using the power heuristic (exponent = 2).
     * @details Weight = (pdf_camera^2) / (pdf_camera^2 + pdf_light^2).
     * @param p_cam The PDF of generating the combined path using the camera sub-path strategy.
     * @param p_light The PDF of generating the combined path using the light sub-path strategy.
     * @return The MIS weight.
     */
    inline double mis_power(double p_cam, double p_light) {
        double wc = p_cam * p_cam;
        double wl = p_light * p_light;
        double sum_weighted_pdfs = wc + wl;
        if (sum_weighted_pdfs < PDF_EPSILON * PDF_EPSILON) return 0.0;
        return wc / sum_weighted_pdfs;
    }

    /**
     * @brief Calculates the MIS weight for a combined path strategy in BDPT.
     * @details This function implements Veach's power heuristic (exponent 2) for combining path sampling strategies.
     * It considers the forward PDFs (pdfFwd) and reverse PDFs (pdfRev) stored in path vertices.
     * @param cam The camera sub-path vertices (y_0, ..., y_s).
     * @param s The number of segments in the camera sub-path being considered for this strategy (connects vertex y_s).
     * @param light The light sub-path vertices (z_0, ..., z_t).
     * @param t The number of segments in the light sub-path being considered for this strategy (connects vertex z_t).
     * @param k_total The total number of segments in the combined path (s + t for direct connection, s + t + 1 for connection strategy).
     * This parameter is currently unused in the provided Veach's reference implementation style but is common in some MIS formulations.
     * @return The MIS weight for the given strategy.
     *
     * @note This implementation is based on Veach's thesis / PBRT's interpretation for BDPT MIS.
     * Ensure `pdfFwd` stores P+(v_i-1 -> v_i) and `pdfRev` stores P-(v_i+1 -> v_i).
     * For path y_0...y_s connected to z_t...z_0, the path vertices are (y_0...y_s, z_t...z_0).
     * s = index of camera vertex (y_s), t = index of light vertex (z_t).
     * Strategy (s,t) connects y_s and z_t. Number of segments in camera path part = s. Number of segments in light path part = t.
     * This function assumes `s` refers to index of last camera vertex, `t` for last light vertex in MIS sense.
     */
    double mis_weight_fixed(const Path& cam_path, size_t s_cam_idx, const Path& light_path, size_t t_light_idx, [[maybe_unused]] int k_total) {
        if (s_cam_idx >= cam_path.size() && t_light_idx >= light_path.size()) return 1.0;

        double p_cam_strategy = 1.0;
        for (size_t i = 0; i <= s_cam_idx && i < cam_path.size(); ++i) {
            p_cam_strategy *= (cam_path[i].pdfFwd + PDF_EPSILON);
        }
        for (size_t i = 0; i <= t_light_idx && i < light_path.size(); ++i) {
            if (i > 0 && i < light_path.size()) {
                 p_cam_strategy *= (light_path[t_light_idx - i].pdfRev + PDF_EPSILON);
            }
        }


        double p_light_strategy = 1.0;
        for (size_t i = 0; i <= t_light_idx && i < light_path.size(); ++i) {
            p_light_strategy *= (light_path[i].pdfFwd + PDF_EPSILON);
        }
        for (size_t i = 0; i <= s_cam_idx && i < cam_path.size(); ++i) {
             if (i > 0 && i < cam_path.size()) {
                p_light_strategy *= (cam_path[s_cam_idx - i].pdfRev + PDF_EPSILON);
             }
        }

        // Power heuristic (beta=2).
        double wc = p_cam_strategy * p_cam_strategy;
        double wl = p_light_strategy * p_light_strategy;
        double denom = wc + wl;

        if (denom < (PDF_EPSILON * PDF_EPSILON) ) return 0.0;
        return wc / denom;
    }


    /**
     * @brief Renders the scene using Bidirectional Path Tracing (BDPT).
     * @details This function orchestrates the BDPT algorithm:
     * 1. For each pixel and each sample:
     * a. Generate a camera sub-path.
     * b. Generate a light sub-path.
     * c. Combine sub-paths using various strategies (camera hits emitter, light hits camera, connect paths).
     * d. Weight contributions using Multiple Importance Sampling (MIS).
     * 2. Accumulate radiance contributions to form the final image.
     * @param scene The scene to render, containing camera, geometry, materials, and lights.
     * @param params Rendering parameters (image dimensions, samples per pixel, max path depth).
     * @param showProgress If true, prints progress updates to the console.
     * @return A vector of RGB color values representing the rendered image framebuffer.
     */
    std::vector<Math::Vector3D> renderBDPT(const RayTracer::Scene& scene,
                                           const Utils::RenderParams& params,
                                           bool showProgress) {
        std::vector<Math::Vector3D> framebuffer(params.W * params.H, Math::Vector3D(0,0,0));
        int max_total_bounces = params.max_depth;

        double exposure_factor = 0.5; // Simple exposure control. Adjust for overall brightness.

        #ifdef _OPENMP
        #pragma omp parallel for schedule(dynamic, 1)
        #endif
        for (int j = 0; j < params.H; ++j) { // Iterate over image rows.
            if (omp_get_thread_num() == 0 && showProgress && (j % 10 == 0 || j == params.H -1) ) { // Progress indicator from one thread.
                std::cerr << "\rScanlines remaining (BDPT): " << (params.H - 1 - j) << " / " << params.H << "    " << std::flush;
            }
            for (int i = 0; i < params.W; ++i) {
                Math::Vector3D pixel_color_accumulator(0,0,0);
                for (int sample_idx = 0; sample_idx < params.spp; ++sample_idx) { // Samples per pixel (SPP).
                    // Generate camera ray with jitter for anti-aliasing.
                    double u_jitter = (static_cast<double>(i) + local_rand01()) / (static_cast<double>(params.W));
                    double v_jitter = (static_cast<double>(params.H - 1 - j) + local_rand01()) / (static_cast<double>(params.H)); // Y is often flipped.
                    RayTracer::Ray cam_ray = scene.camera.ray(u_jitter, v_jitter);

                    // --- Generate Camera Sub-path (y_0, ..., y_s_max) ---
                    Path camera_path_vertices;
                    generate_subpath_fixed(scene, cam_ray, max_total_bounces, camera_path_vertices, true, params);

                    // --- Generate Light Sub-path (z_0, ..., z_t_max) ---
                    RayTracer::Ray initial_light_ray;
                    Math::Vector3D Le_raw_sampled_light_intensity;
                    double pdf_pos_area_light = 0.0;
                    double pdf_emit_dir_solid_angle = 0.0;
                    RayTracer::Vertex light_emitter_vertex;

                    bool light_source_was_sampled = sample_light_source_fixed(scene, initial_light_ray, Le_raw_sampled_light_intensity,
                                                                              pdf_pos_area_light, pdf_emit_dir_solid_angle, light_emitter_vertex);

                    Path light_path_from_source_segments;
                    Path full_light_path_vertices;

                    if (light_source_was_sampled) {
                        double combined_initial_light_pdf_A_times_W = pdf_pos_area_light * pdf_emit_dir_solid_angle;
                        if (combined_initial_light_pdf_A_times_W < PDF_EPSILON) {
                            light_emitter_vertex.beta = Math::Vector3D(0,0,0);
                        } else {
                            light_emitter_vertex.beta = Le_raw_sampled_light_intensity / combined_initial_light_pdf_A_times_W;
                        }
                        light_emitter_vertex.pdfFwd = combined_initial_light_pdf_A_times_W;
                        light_emitter_vertex.pdfRev = 0;
                        full_light_path_vertices.push_back(light_emitter_vertex);
                        if (light_emitter_vertex.beta.dot(light_emitter_vertex.beta) > THROUGHPUT_EPSILON_SQ) {
                            generate_subpath_fixed(scene, initial_light_ray, max_total_bounces - 1,
                                                   light_path_from_source_segments, false, params);

                            for(const auto& segment_v_data : light_path_from_source_segments) {
                                RayTracer::Vertex actual_segment_v = segment_v_data;
                                actual_segment_v.beta = light_emitter_vertex.beta * segment_v_data.beta; // Accumulate throughput: beta(z_0...z_i) = beta(z_0) * beta(z_1...z_i).
                                full_light_path_vertices.push_back(actual_segment_v);
                            }
                        }
                    }
                    Math::Vector3D L_for_sample(0,0,0);
                    // === Accumulate Contributions from Different BDPT Strategies ===
                    for (size_t s_idx = 0; s_idx < camera_path_vertices.size(); ++s_idx) {
                        const auto& cam_v = camera_path_vertices[s_idx];
                        Math::Vector3D emission_val = cam_v.rec.material.emission * cam_v.rec.material.emissionStrength;
                        if (emission_val.dot(emission_val) > GEOMETRY_EPSILON) {
                            double w = mis_weight_fixed(camera_path_vertices, s_idx, Path{}, (size_t)-1, s_idx);
                            L_for_sample += cam_v.beta * emission_val * w;
                        }
                    }

                    if (!camera_path_vertices.empty() && !full_light_path_vertices.empty()) {
                        for (size_t s_idx = 0; s_idx < camera_path_vertices.size(); ++s_idx) {
                            const auto& cam_v = camera_path_vertices[s_idx];
                            if (cam_v.delta || cam_v.rec.t == std::numeric_limits<double>::infinity()) continue;

                            const auto& light_v_emitter = full_light_path_vertices[0];
                            if (light_v_emitter.beta.dot(light_v_emitter.beta) < THROUGHPUT_EPSILON_SQ) continue;
                            double G = geometry_term_fixed(scene, cam_v, light_v_emitter);
                            if (G < GEOMETRY_EPSILON) continue;

                            Math::Vector3D dir_ys_to_z0 = safe_normalize(light_v_emitter.rec.point - cam_v.rec.point);
                            Math::Vector3D bsdf_at_ys = eval_bsdf(cam_v.rec, cam_v.wi /* outgoing from y_s towards camera */,
                                                                  dir_ys_to_z0 /* incoming to y_s from z_0 */,
                                                                  cam_v.rec.normal, false);
                            Math::Vector3D L_connect_explicit = cam_v.beta * bsdf_at_ys * G * light_v_emitter.beta;

                            // MIS weight for strategy (s,1) using Veach's notation (s camera edges, 1 implicit light edge z_0 to eye).
                            double w_connect_explicit = mis_weight_fixed(camera_path_vertices, s_idx, full_light_path_vertices, 0, s_idx + 1);
                            L_for_sample += L_connect_explicit * w_connect_explicit;
                        }
                        for (size_t s_idx = 0; s_idx < camera_path_vertices.size(); ++s_idx) {
                            const auto& cam_v = camera_path_vertices[s_idx]; // Vertex y_s.
                            if (cam_v.delta || cam_v.rec.t == std::numeric_limits<double>::infinity()) continue; // Cannot connect from delta/sky.
                            for (size_t t_idx = 1; t_idx < full_light_path_vertices.size(); ++t_idx) {
                                const auto& light_v = full_light_path_vertices[t_idx];
                                if (light_v.delta || light_v.rec.t == std::numeric_limits<double>::infinity()) continue;
                                if (light_v.beta.dot(light_v.beta) < THROUGHPUT_EPSILON_SQ) continue;

                                double G = geometry_term_fixed(scene, cam_v, light_v);
                                if (G < GEOMETRY_EPSILON) continue;
                                Math::Vector3D dir_ys_to_zt = safe_normalize(light_v.rec.point - cam_v.rec.point);
                                Math::Vector3D bsdf_at_ys = eval_bsdf(cam_v.rec, cam_v.wi, dir_ys_to_zt, cam_v.rec.normal, false);
                                Math::Vector3D bsdf_at_zt = eval_bsdf(light_v.rec, light_v.wi, dir_ys_to_zt * -1.0, light_v.rec.normal, false);

                                Math::Vector3D L_connect = cam_v.beta * bsdf_at_ys * G * bsdf_at_zt * light_v.beta;
                                double w = mis_weight_fixed(camera_path_vertices, s_idx, full_light_path_vertices, t_idx, s_idx + t_idx + 1);
                                L_for_sample += L_connect * w;
                            }
                        }
                    }
                    pixel_color_accumulator += L_for_sample;
                }

                Math::Vector3D final_color_hdr = pixel_color_accumulator / static_cast<double>(params.spp);
                final_color_hdr = final_color_hdr * exposure_factor; // Apply exposure.

                // Reinhard tone mapping.
                Math::Vector3D final_color_tm = final_color_hdr / (final_color_hdr + Math::Vector3D(1.0, 1.0, 1.0));

                // Simple gamma correction (approximate sRGB).
                final_color_tm.x = std::pow(final_color_tm.x, 1.0/2.2);
                final_color_tm.y = std::pow(final_color_tm.y, 1.0/2.2);
                final_color_tm.z = std::pow(final_color_tm.z, 1.0/2.2);

                // Clamp color values to [0,1]
                final_color_tm.x = std::max(0.0, std::min(1.0, final_color_tm.x));
                final_color_tm.y = std::max(0.0, std::min(1.0, final_color_tm.y));
                final_color_tm.z = std::max(0.0, std::min(1.0, final_color_tm.z));

                framebuffer[j * params.W + i] = final_color_tm;
            }
        }
        if (showProgress) std::cerr << "\nBDPT rendering finished." << std::endl;
        return framebuffer;
    }

} // namespace BDPT

#endif // BDPT_HPP