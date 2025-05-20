/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/core/Utils.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

float Utils::clamp01(float v) {
    return v < 0.f ? 0.f : (v > 1.f ? 1.f : v);
}

Math::Point3D Utils::orbit(const Math::Point3D& center, double radius,
    double yawDeg, double pitchDeg)
{
    double yaw   = yawDeg   * M_PI/180.0;
    double pitch = pitchDeg * M_PI/180.0;
    return {
    center.x + radius * std::cos(pitch) * std::sin(yaw),
    center.y + radius * std::sin(pitch),
    center.z + radius * std::cos(pitch) * std::cos(yaw)
    };
}

bool Utils::checkUsage(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <scene.cfg>\n";
        std::cerr << "\t - [-t N]        Spawn N workers for parallel rendering\n";
        std::cerr << "\t - [-p]          Displays a progress bar and rendering metrics\n";
        std::cerr << "\t - [-f]          Renders at half base settings for previews\n";
        std::cerr << "\t - [-viz [mode]  Open a window to visualize the render process\n";
        std::cerr << "\t\t\t Available modes are :\n";
        std::cerr << "\t\t\t\t scanline   -- line-by-line drawing\n";
        std::cerr << "\t\t\t\t tiles      -- block-based rendering\n";
        std::cerr << "\t\t\t\t random     -- random pixel sampling\n";
        std::cerr << "\t - [-d] show Debug infos\n";
        return false;
    }
    return true;
}

int Utils::parseThreads(int argc, char* argv[]) {
    for (int i = 2; i + 1 < argc; ++i)
        if (std::string(argv[i]) == "-t") {
            int wanted_thread = std::stoi(argv[i + 1]);
            if (wanted_thread <= 0)
                throw std::exception();
            return wanted_thread;
        }
    return 1;
}

bool Utils::parseProgress(int argc, char* argv[]) {
    for (int i = 2; i < argc; ++i)
        if (std::string(argv[i]) == "-p")
            return true;
    return false;
}

Utils::RenderParams Utils::getRenderParams(int argc, char* argv[]) {
    bool fast = false;
    int sppVal = 32;
    int mdVal  = 16;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f") {
            fast = true;
        } else if (arg == "-spp" && i + 1 < argc) {
            sppVal = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "-md" && i + 1 < argc) {
            mdVal = std::max(1, std::atoi(argv[++i]));
        }
    }
    if (fast) {
        sppVal = std::max(1, sppVal / 2);
        mdVal  = std::max(1, mdVal  / 2);
    }
    RenderParams p;
    p.W = 1000;
    p.H = 800;
    p.spp = sppVal;
    p.max_depth = mdVal;
    p.fast = fast;
    return p;
}

bool Utils::isDebugModeEnable(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-d") {
            return true;
        }
    }
    return false;
}

void Utils::printSettings(const Utils::RenderParams& p, bool debug) {
    if (debug) {
        std::cerr << "############### SETTINGS #############\n"
                << "\tresolution: " << p.W << "x" << p.H
                << "\tspp: " << p.spp << " | max_depth: " << p.max_depth << "\n"
                << "######################################\n";
    }
}

bool Utils::loadSceneOrExit(const char* cfg, RayTracer::Scene& scene, bool debug) {
    if (!RayTracer::loadScene(cfg, scene, debug)) {
        return false;
    }
    return true;
}

RayTracer::Camera Utils::createCamera(const Utils::RenderParams& p) {
    return RayTracer::Camera(
            {0,3,6}, {0,0,0}, {0,1,0},
            45.0, double(p.W) / p.H
    );
}

void Utils::PrintProgressBar(int percent,
                             long long totalRays,
                             const std::chrono::steady_clock::time_point &start) {
    constexpr int width = 30;
    int pos = percent * width / 100;

    auto now     = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - start).count();
    long long raysDone = totalRays * percent / 100;
    double rps = elapsed > 0 ? raysDone / elapsed : 0.0;

    double countScale;
    const char* countUnit;
    if (totalRays >= 1e9) { countScale = 1e9; countUnit = "Brays"; }
    else if (totalRays >= 1e6) { countScale = 1e6; countUnit = "Mrays"; }
    else { countScale = 1e3; countUnit = "Krays"; }

    double doneCount = raysDone / countScale;
    double totalCount = totalRays / countScale;

    double rateScale;
    const char* rateUnit;
    if (rps >= 1e9) { rateScale = 1e9;  rateUnit = "Brays/s"; }
    else if (rps >= 1e6) { rateScale = 1e6;  rateUnit = "Mrays/s"; }
    else { rateScale = 1e3;  rateUnit = "Krays/s"; }

    double displayRate = rps / rateScale;

    // Bar
    std::cerr << "\r[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) {
            double frac = double(i) / (width - 1);
            int r = 255, g = int(255 * (1 - frac)), b = int(255 * frac);
            std::cerr << "\x1b[38;2;" << r << ";" << g << ";" << b << "m" << "=";
        } else {
            std::cerr << " ";
        }
    }
    std::cerr << "\x1b[0m] "
              << std::setw(3) << percent << "%  "
              << std::fixed << std::setprecision(2)
              << doneCount << "/" << totalCount << " " << countUnit << "  "
              << displayRate << " " << rateUnit
              << std::flush;
}

    std::vector<Math::Vector3D>
    Utils::renderImage(const RayTracer::Scene& scene,
                       const Utils::RenderParams& p,
                       bool showProgress)
    {
        int w = p.W, h = p.H, spp = p.spp, maxd = p.max_depth;
        long long totalRays = 1LL * w * h * spp;          // total work
        std::vector<Math::Vector3D> fb(w * h);
        int grid = static_cast<int>(std::sqrt(spp) + 0.5);
        double inv_spp = 1.0 / spp;

        using clk = std::chrono::steady_clock;
        auto start = clk::now();
        int next_pct = 1;

        for (int j = 0; j < h; ++j) {
#pragma omp parallel for schedule(dynamic)
            for (int i = 0; i < w; ++i) {
                Math::Vector3D col{0,0,0};
                for (int s = 0; s < spp; ++s) {
                    int gx = s % grid, gy = s / grid;
                    double u = (i + (gx+0.5)/grid) / (w - 1);
                    double v = (j + (gy+0.5)/grid) / (h - 1);
                    auto ray = scene.camera.ray(u, 1.0 - v);
                    col += RayTracer::shade(ray, scene, 0, maxd);
                }
                fb[j * w + i] = col * inv_spp;
            }

            if (showProgress) {
                int pct = (j + 1) * 100 / h;
                while (pct >= next_pct && next_pct <= 100) {
                    PrintProgressBar(next_pct++, totalRays, start);
                }
            }
        }
        if (showProgress) std::cerr << "\n";
        return fb;
    }

Math::Vector3D Utils::adjustContrast(const Math::Vector3D& c, float contrast) {
    return {
            clamp01((c.x - 0.5f) * contrast + 0.5f),
            clamp01((c.y - 0.5f) * contrast + 0.5f),
            clamp01((c.z - 0.5f) * contrast + 0.5f)
    };
}

Math::Vector3D Utils::adjustSaturation(const Math::Vector3D& c, float satF) {
    double r = clamp01((float)c.x), g = clamp01((float)c.y), b = clamp01((float)c.z);
    double maxv = std::max({r,g,b}), minv = std::min({r,g,b});
    double d = maxv - minv;
    if (d < 1e-6) return c;
    double v = maxv, s = std::min(d / maxv * satF, 1.0);
    double h;
    if (r == maxv) h = (g - b) / d;
    else if (g == maxv) h = 2.0 + (b - r) / d;
    else h = 4.0 + (r - g) / d;
    h = std::fmod((h * 60.0 + 360.0), 360.0);
    double C = v * s;
    double X = C * (1.0 - std::fabs(std::fmod(h/60.0, 2.0) - 1.0));
    double m = v - C;
    double rp, gp, bp;
    if (h < 60) { rp = C;  gp = X;  bp = 0; }
    else if (h < 120) { rp = X;  gp = C;  bp = 0; }
    else if (h < 180) { rp = 0;  gp = C;  bp = X; }
    else if (h < 240) { rp = 0;  gp = X;  bp = C; }
    else if (h < 300) { rp = X;  gp = 0;  bp = C; }
    else  { rp = C; gp = 0; bp = X; }
    return { clamp01((float)(rp + m)), clamp01((float)(gp + m)), clamp01((float)(bp + m)) };
}

void Utils::postProcess(std::vector<Math::Vector3D>& fb, float contrast, float saturation) {
    for (auto& c : fb) {
        c = adjustContrast(c, contrast);
        c = adjustSaturation(c, saturation);
    }
}

void Utils::writePPM(const std::vector<Math::Vector3D>& fb, int w, int h) {
    std::cout << "P3\n" << w << ' ' << h << "\n255\n";
    double gamma = 1.0 / 2.2;
    for (const auto& c : fb) {
        int ir = static_cast<int>(255.999 * std::pow(c.x, gamma));
        int ig = static_cast<int>(255.999 * std::pow(c.y, gamma));
        int ib = static_cast<int>(255.999 * std::pow(c.z, gamma));
        std::cout << ir << ' ' << ig << ' ' << ib << '\n';
    }
}