/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "rendering/ShadowShading.hpp"
#include "../../include/rendering/Skybox.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>

bool Skybox::PPMImage::isLoaded() const {
    return width > 0 && height > 0 && !data.empty();
}

Math::Vector3D Skybox::PPMImage::getColorAtTexel(int x, int y) const {
    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);

    size_t idx = (static_cast<size_t>(y) * width + x) * 3;

    if (idx + 2 >= data.size()) {
        std::cerr << "SKYBOX GET_COLOR_AT_TEXEL ERROR: idx=" << idx
                  << " for x=" << x << ", y=" << y << " out of range (size: " << data.size() << ")\n";
        return {1.0, 0.0, 1.0};
    }
    return {
        data[idx]     / 255.0,
        data[idx + 1] / 255.0,
        data[idx + 2] / 255.0
    };
}

Math::Vector3D Skybox::PPMImage::sample(double u, double v) const {
    if (!isLoaded()) {
        return {0.1, 0.1, 0.2};
    }

    u = u - std::floor(u);
    v = std::clamp(v, 0.0, 1.0);

    double fx = u * (width - 1.0);
    double fy = v * (height - 1.0);

    int x1 = static_cast<int>(std::floor(fx));
    int y1 = static_cast<int>(std::floor(fy));

    double u_ratio = fx - x1;
    double v_ratio = fy - y1;

    Math::Vector3D c00 = getColorAtTexel(x1, y1);
    Math::Vector3D c10 = getColorAtTexel(x1 + 1, y1);
    Math::Vector3D c01 = getColorAtTexel(x1, y1 + 1);
    Math::Vector3D c11 = getColorAtTexel(x1 + 1, y1 + 1);

    Math::Vector3D top_interp = c00 * (1.0 - u_ratio) + c10 * u_ratio;
    Math::Vector3D bottom_interp = c01 * (1.0 - u_ratio) + c11 * u_ratio;

    return top_interp * (1.0 - v_ratio) + bottom_interp * v_ratio;
}

Skybox::PPMImage Skybox::detail::globalSkyboxTexture;

bool Skybox::loadPPMFile(const std::string& filepath, PPMImage& image) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "SKYBOX ERROR: Could not open PPM file: " << filepath << std::endl;
        return false;
    }
    std::string magic;
    ifs >> magic;
    if (!ifs) return false;

    bool isP3 = (magic == "P3");
    if (!isP3 && magic != "P6") return false;

    auto skip_ws_comments = [&]() {
        while (true) {
            int c = ifs.peek();
            if (c == EOF) return false;
            if (std::isspace(static_cast<unsigned char>(c))) { ifs.get(); }
            else if (c == '#') { std::string comment; std::getline(ifs, comment); }
            else break;
        }
        return ifs.good();
    };

    if (!skip_ws_comments()) return false;
    int w, h, maxv;
    ifs >> w; if (!ifs || !skip_ws_comments()) return false;
    ifs >> h; if (!ifs || !skip_ws_comments()) return false;
    ifs >> maxv; if (!ifs || maxv != 255) return false;

    image.width = w;
    image.height = h;
    image.data.resize(static_cast<size_t>(w) * h * 3);

    if (isP3) {
        for (size_t i = 0; i < image.data.size(); ++i) {
            int v_int;
            ifs >> v_int;
            if (!ifs) return false;
            image.data[i] = static_cast<unsigned char>(std::clamp(v_int, 0, 255));
        }
    } else {
        char single_whitespace;
        ifs.get(single_whitespace);
        if (!std::isspace(static_cast<unsigned char>(single_whitespace))) return false;
        ifs.read(reinterpret_cast<char*>(image.data.data()), image.data.size());
        if (static_cast<size_t>(ifs.gcount()) != image.data.size()) return false;
    }
    return true;
}

bool Skybox::initializeSkybox(const std::string& filepath) {
    return loadPPMFile(filepath, detail::globalSkyboxTexture);
}

Math::Vector3D Skybox::skyGradient(const Math::Vector3D& unitDir) {
    if (!detail::globalSkyboxTexture.isLoaded()) {
        double t = 0.5 * (unitDir.y + 1.0);
        t = std::clamp(t, 0.0, 1.0);
        t = t * t * (3.0 - 2.0 * t);

        Math::Vector3D bottomColor{0.7, 0.8, 1.0};
        Math::Vector3D topColor{0.2, 0.3, 0.8};
        Math::Vector3D col = bottomColor * (1.0 - t) + topColor * t;

        double sunIntensity = std::pow(std::max(0.0, unitDir.y), 8.0);
        Math::Vector3D sunColor{0.3, 0.25, 0.2};
        col = col + sunColor * sunIntensity;
        col.x = std::min(col.x, 1.0);
        col.y = std::min(col.y, 1.0);
        col.z = std::min(col.z, 1.0);
        return col;
    }

    double y_clamped = std::clamp(unitDir.y, -1.0, 1.0);

    double phi    = std::atan2(unitDir.z, unitDir.x);
    double lat    = std::asin(y_clamped);

    double u_raw  = (phi + M_PI) / (2.0 * M_PI);
    double v_temp  = (lat + M_PI_2) / M_PI;
    double u_final = u_raw;
    double v_final  = 1.0 - v_temp;

    return detail::globalSkyboxTexture.sample(u_final, v_final);
}


Math::Vector3D RayTracer::shade(const RayTracer::Ray &ray,
                                const RayTracer::Scene &scene,
                                int depth,
                                int maxDepth)
{
    const double EPS = 1e-4;
    if (depth >= maxDepth) return {0,0,0};

    double tHit = std::numeric_limits<double>::infinity();
    RayTracer::HitRecord rec;

    if (!scene.hit(ray, tHit, rec)) {
        auto dir = ray.direction.normalized();
        return Skybox::skyGradient(dir);
    }

    const RayTracer::Material &M = rec.material;
    Math::Vector3D direct_emission = M.emission * M.emissionStrength;

    Math::Vector3D albedo_val = M.diffuseMap
                                ? M.diffuseMap->sample(rec.uv.x, rec.uv.y)
                                : M.albedo;
    if (albedo_val.x > 1.01 || albedo_val.y > 1.01 || albedo_val.z > 1.01)
        albedo_val *= (1.0/255.0);

    Math::Vector3D N_geom = rec.normal;
    Math::Vector3D N_shading = N_geom;

    const double roughness_threshold = 0.05;
    const double metallic_threshold = 0.5;
    const double transparency_threshold = 0.01;
    const double ior_threshold = 1.0001;

    bool mat_is_smooth_dielectric = M.metallic < metallic_threshold &&
                                    M.roughness < roughness_threshold &&
                                    M.transparency > transparency_threshold &&
                                    M.ior > ior_threshold;
    bool mat_is_metal = M.metallic >= metallic_threshold;
    bool mat_is_smooth_metal = mat_is_metal && M.roughness < roughness_threshold;

    Math::Vector3D direct_lighting = {0,0,0};
    for (auto const &light : scene.lights) {
        Math::Vector3D light_contrib = {0,0,0};
        if (light.type == RayTracer::LightType::Ambient) {
            light_contrib = albedo_val * light.intensity * (1.0 - M.metallic);
            direct_lighting += light_contrib;
            continue;
        }

        Math::Vector3D Ld;
        double lightDist = std::numeric_limits<double>::infinity();

        if (light.type == RayTracer::LightType::Directional) {
            Ld = light.direction * -1.0;
        } else {
            auto delta = light.position - rec.point;
            lightDist = delta.length();
            if (lightDist < EPS) continue;
            Ld = delta * (1.0/lightDist);
        }

        double NdotL = std::max(0.0, N_shading.dot(Ld));
        if (NdotL <= 0.0) continue;

        RayTracer::Ray shadow_ray{rec.point + N_shading*EPS, Ld};
        double shadow_transparency = 1.0;
        for (auto const &obj_shadow : scene.objects) {
            double t_shadow; RayTracer::HitRecord hr_shadow;
            if (obj_shadow->hits(shadow_ray, t_shadow, hr_shadow) && t_shadow < lightDist && t_shadow > EPS) {
                shadow_transparency *= hr_shadow.material.transparency;
                if (shadow_transparency < 1e-3) break;
            }
        }
        if (shadow_transparency < 1e-3) continue;

        Math::Vector3D brdf_direct_val = {0,0,0};
        if (!mat_is_smooth_dielectric && !mat_is_smooth_metal)
            brdf_direct_val = albedo_val / M_PI;

        double light_dist_attenuation = (light.type == RayTracer::LightType::Directional)
                                        ? 1.0
                                        : 1.0 / (lightDist * lightDist + 1e-8);

        if (brdf_direct_val.x > 0 || brdf_direct_val.y > 0 || brdf_direct_val.z > 0) {
            light_contrib = brdf_direct_val * light.intensity * NdotL * light_dist_attenuation * shadow_transparency;
            direct_lighting += light_contrib;
        }
    }
    direct_lighting += direct_emission;

    double max_albedo_comp = std::max({albedo_val.x, albedo_val.y, albedo_val.z});
    double rr_prob_basis = mat_is_smooth_dielectric ? std::max(static_cast<double>(M.transparency), 0.75)
                                                    : max_albedo_comp;
    double p_cont  = std::min(0.95, std::max(rr_prob_basis, 0.05));
    if (depth > 1 && rand01() > p_cont) return direct_lighting;

    Math::Vector3D indirect_lighting = {0,0,0};
    bool entering = ray.direction.dot(N_geom) < 0;
    Math::Vector3D n_interface = entering ? N_shading : (N_shading * -1.0);

    RayTracer::Ray scattered_ray_obj;
    Math::Vector3D bsdf_attenuation_val;
    double bsdf_path_pdf = 1.0;

    if (mat_is_smooth_dielectric) {
        double cosi = ray.direction.dot(n_interface);
        double etai = entering ? 1.0 : M.ior;
        double etat = entering ? M.ior : 1.0;
        double eta_ratio = etai / etat;
        double k_snell = 1.0 - eta_ratio * eta_ratio * (1.0 - cosi * cosi);

        double fresnel_reflectance_value;
        if (k_snell < 0.0) {
            fresnel_reflectance_value = 1.0;
        } else {
            double cos_theta1_abs = std::abs(cosi);
            double cos_theta2_abs = std::sqrt(k_snell);
            double rs_num = etai * cos_theta1_abs - etat * cos_theta2_abs;
            double rs_den = etai * cos_theta1_abs + etat * cos_theta2_abs;
            double Rs = (std::abs(rs_den) < 1e-8) ? 1.0 : (rs_num / rs_den); Rs *= Rs;
            double rp_num = etai * cos_theta2_abs - etat * cos_theta1_abs;
            double rp_den = etai * cos_theta2_abs + etat * cos_theta1_abs;
            double Rp = (std::abs(rp_den) < 1e-8) ? 1.0 : (rp_num / rp_den); Rp *= Rp;
            fresnel_reflectance_value = 0.5 * (Rs + Rp);
        }

        fresnel_reflectance_value = std::clamp(fresnel_reflectance_value, 0.0, 1.0);
        if (rand01() < fresnel_reflectance_value) {
            Math::Vector3D R_dir = ray.direction - n_interface * 2.0 * ray.direction.dot(n_interface);
            double r_dir_len = R_dir.length(); if (r_dir_len > 1e-8) R_dir *= (1.0 / r_dir_len);
            scattered_ray_obj = RayTracer::Ray{rec.point + n_interface * EPS, R_dir};
            bsdf_attenuation_val = {1.0, 1.0, 1.0};
            bsdf_path_pdf = fresnel_reflectance_value;
        } else {
            Math::Vector3D T_dir = ray.direction * eta_ratio + n_interface * (eta_ratio * cosi - std::sqrt(k_snell));
            double t_dir_len = T_dir.length(); if (t_dir_len > 1e-8) T_dir *= (1.0 / t_dir_len);
            scattered_ray_obj = RayTracer::Ray{rec.point - n_interface * EPS, T_dir};
            bsdf_attenuation_val = albedo_val;
            bsdf_path_pdf = 1.0 - fresnel_reflectance_value;
        }
    } else if (mat_is_metal) {
        Math::Vector3D R_dir = ray.direction - N_shading * 2.0 * ray.direction.dot(N_shading);
        double r_dir_len = R_dir.length(); if (r_dir_len > 1e-8) R_dir *= (1.0 / r_dir_len);
        scattered_ray_obj = RayTracer::Ray{rec.point + N_shading * EPS, R_dir};
        Math::Vector3D F0 = M.specular;  // e.g. {1.00,0.766,0.336}
        bsdf_attenuation_val = F0;
    } else {
        double hemisphere_pdf;
        Math::Vector3D D = RayTracer::sampleHemisphere(N_shading, hemisphere_pdf);
        scattered_ray_obj = RayTracer::Ray{rec.point + N_shading * EPS, D};
        bsdf_attenuation_val = albedo_val;
        bsdf_path_pdf = hemisphere_pdf;
    }

    Math::Vector3D Li = RayTracer::shade(scattered_ray_obj, scene, depth + 1, maxDepth);
    if (mat_is_smooth_dielectric || mat_is_metal) {
        if (bsdf_path_pdf > 1e-7)
            indirect_lighting = bsdf_attenuation_val * Li / (bsdf_path_pdf * p_cont + 1e-8);
    } else {
        indirect_lighting = bsdf_attenuation_val * Li / (p_cont + 1e-8);
    }

    Math::Vector3D result = direct_lighting + indirect_lighting;
    result.x = RayTracer::clamp01(result.x);
    result.y = RayTracer::clamp01(result.y);
    result.z = RayTracer::clamp01(result.z);
    return result;
}
