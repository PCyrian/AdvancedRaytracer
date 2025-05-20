/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/core/Visualizer.hpp"
#include "../../include/core/Utils.hpp"
#include "../../include/core/Vector3D.hpp"
#include "../../include/rendering/ShadowShading.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <algorithm>
#include <numeric>
#include <omp.h>
#include <chrono>
#include <iomanip>
#include <thread>
#ifdef USE_SFML
#include <SFML/Graphics.hpp>
#endif

namespace Viz {
    namespace {
        inline uint8_t toByte(float v) {
            return static_cast<uint8_t>(255.0f * Utils::clamp01(v));
        }

        void renderScanline(const RayTracer::Scene& scene,
                            const Utils::RenderParams& p,
                            sf::RenderWindow& window,
                            sf::Texture& texture,
                            const sf::Sprite& sprite,
                            std::vector<Math::Vector3D>& fb,
                            std::vector<sf::Uint8>& pixels,
                            bool showProgress,
                            long long totalRays,
                            const std::chrono::steady_clock::time_point& start)
        {
            const int w = p.W, h = p.H, spp = p.spp, maxd = p.max_depth;
            const int batch = 8;
            long long raysProcessed = 0;
            for (int j = 0; j < h && window.isOpen(); ++j) {
                #pragma omp parallel for schedule(dynamic)
                for (int i = 0; i < w; ++i) {
                    Math::Vector3D col{0,0,0};
                    for (int s = 0; s < spp; ++s) {
                        double u = (i + 0.5) / (w - 1);
                        double v = (j + 0.5) / (h - 1);
                        auto ray = scene.camera.ray(u, 1.0 - v);
                        col += RayTracer::shade(ray, scene, 0, maxd);
                    }
                    Math::Vector3D avg = col * (1.0 / spp);
                    fb[j * w + i] = avg;
                    size_t idx = 4 * (j * w + i);
                    pixels[idx+0] = toByte(avg.x);
                    pixels[idx+1] = toByte(avg.y);
                    pixels[idx+2] = toByte(avg.z);
                    pixels[idx+3] = 255;
                }
                raysProcessed += static_cast<long long>(w) * spp;
                if (showProgress) {
                    int percent = int(raysProcessed * 100 / totalRays);
                    Utils::PrintProgressBar(percent, totalRays, start);
                }
                texture.update(pixels.data() + j * w * 4, w, 1, 0, j);
                if ((j % batch) == 0 || j == h - 1) {
                    window.clear();
                    window.draw(sprite);
                    window.display();
                }
                sf::Event e;
                while (window.pollEvent(e))
                    if (e.type == sf::Event::Closed)
                        return;
            }
        }

        void renderTiles(const RayTracer::Scene& scene,
                         const Utils::RenderParams& p,
                         sf::RenderWindow& window,
                         sf::Texture& texture,
                         const sf::Sprite& sprite,
                         std::vector<Math::Vector3D>& fb,
                         std::vector<sf::Uint8>& pixels,
                         bool showProgress,
                         long long totalRays,
                         const std::chrono::steady_clock::time_point& start)
        {
            const int w = p.W, h = p.H, spp = p.spp, maxd = p.max_depth;
            const int tileSize = 64;
            long long raysProcessed = 0;
            for (int ty = 0; ty < h && window.isOpen(); ty += tileSize) {
                for (int tx = 0; tx < w && window.isOpen(); tx += tileSize) {
                    int bx = std::min(tileSize, w - tx);
                    int by = std::min(tileSize, h - ty);
                    std::vector<sf::Uint8> tilePixels(bx * by * 4);

                    #pragma omp parallel for collapse(2) schedule(dynamic)
                    for (int y = 0; y < by; ++y) {
                        for (int x = 0; x < bx; ++x) {
                            int i = tx + x, j = ty + y;
                            Math::Vector3D col{0,0,0};
                            for (int s = 0; s < spp; ++s) {
                                double u = (i + 0.5) / (w - 1);
                                double v = (j + 0.5) / (h - 1);
                                auto ray = scene.camera.ray(u, 1.0 - v);
                                col += RayTracer::shade(ray, scene, 0, maxd);
                            }
                            Math::Vector3D avg = col * (1.0 / spp);
                            fb[j * w + i] = avg;

                            size_t gIdx = 4 * (j * w + i);
                            pixels[gIdx+0] = toByte(avg.x);
                            pixels[gIdx+1] = toByte(avg.y);
                            pixels[gIdx+2] = toByte(avg.z);
                            pixels[gIdx+3] = 255;

                            size_t tIdx = 4 * (y * bx + x);
                            tilePixels[tIdx+0] = toByte(avg.x);
                            tilePixels[tIdx+1] = toByte(avg.y);
                            tilePixels[tIdx+2] = toByte(avg.z);
                            tilePixels[tIdx+3] = 255;
                        }
                    }

                    raysProcessed += static_cast<long long>(bx) * by * spp;
                    if (showProgress) {
                        int percent = int(raysProcessed * 100 / totalRays);
                        Utils::PrintProgressBar(percent, totalRays, start);
                    }

                    texture.update(tilePixels.data(), bx, by, tx, ty);
                    window.clear();
                    window.draw(sprite);
                    window.display();

                    sf::Event e;
                    while (window.pollEvent(e))
                        if (e.type == sf::Event::Closed)
                            return;
                }
            }
        }

        void renderRandom(const RayTracer::Scene& scene,
                          const Utils::RenderParams& p,
                          sf::RenderWindow& window,
                          sf::Texture& texture,
                          const sf::Sprite& sprite,
                          std::vector<Math::Vector3D>& fb,
                          std::vector<sf::Uint8>& pixels,
                          bool showProgress,
                          long long totalRays,
                          const std::chrono::steady_clock::time_point& start)
        {
            const int w = p.W, h = p.H, spp = p.spp, maxd = p.max_depth;
            std::vector<int> idxs(w * h);
            std::iota(idxs.begin(), idxs.end(), 0);
            std::shuffle(idxs.begin(), idxs.end(), std::mt19937{std::random_device{}()});
            const int batch = 1024;
            long long raysProcessed = 0;
            int count = 0;

            for (int idx : idxs) {
                int j = idx / w, i = idx % w;
                Math::Vector3D col{0,0,0};
                for (int s = 0; s < spp; ++s) {
                    double u = (i + 0.5) / (w - 1);
                    double v = (j + 0.5) / (h - 1);
                    auto ray = scene.camera.ray(u, 1.0 - v);
                    col += RayTracer::shade(ray, scene, 0, maxd);
                }
                Math::Vector3D avg = col * (1.0 / spp);
                fb[j * w + i] = avg;

                size_t pidx = 4 * idx;
                pixels[pidx+0] = toByte(avg.x);
                pixels[pidx+1] = toByte(avg.y);
                pixels[pidx+2] = toByte(avg.z);
                pixels[pidx+3] = 255;
                texture.update(pixels.data() + pidx, 1, 1, i, j);

                raysProcessed += spp;
                ++count;
                if (showProgress && (count % 10000) == 0) {
                    int percent = int(raysProcessed * 100 / totalRays);
                    Utils::PrintProgressBar(percent, totalRays, start);
                }

                if (count % batch == 0 || count == w * h) {
                    window.clear();
                    window.draw(sprite);
                    window.display();
                }

                sf::Event e;
                while (window.pollEvent(e))
                    if (e.type == sf::Event::Closed)
                        return;
            }
        }
    } // namespace

    std::vector<Math::Vector3D> renderImageSFML(
        const RayTracer::Scene& scene,
        const Utils::RenderParams& p,
        VizMode mode,
        bool showProgress
    ) {
        const int w = p.W, h = p.H, spp = p.spp;
        std::vector<Math::Vector3D> fb(w * h);
        sf::RenderWindow window(sf::VideoMode(w, h), "RayTrace Viz");
        window.setFramerateLimit(60);
        sf::Texture texture; texture.create(w, h);
        sf::Sprite sprite(texture);
        std::vector<sf::Uint8> pixels(w * h * 4, 0);

        window.clear(sf::Color::Black);
        window.display();

        auto start = std::chrono::steady_clock::now();
        long long totalRays = static_cast<long long>(w) * h * spp;

        if      (mode == VizMode::SCANLINE) renderScanline(scene, p, window, texture, sprite, fb, pixels, showProgress, totalRays, start);
        else if (mode == VizMode::TILES)    renderTiles (scene, p, window, texture, sprite, fb, pixels, showProgress, totalRays, start);
        else if (mode == VizMode::RANDOM)   renderRandom(scene, p, window, texture, sprite, fb, pixels, showProgress, totalRays, start);

        // Wait
        while (window.isOpen()) {
            sf::Event e;
            while (window.pollEvent(e))
                if (e.type == sf::Event::Closed)
                    window.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return fb;
    }

    VizMode parseVizMode(int argc, char* argv[]) {
        VizMode mode = VizMode::DEFAULT;

        for (int i = 1; i + 1 < argc; ++i) {
            std::string flag = argv[i];
            std::string lowflag(flag.size(), '\0');
            std::transform(flag.begin(), flag.end(), lowflag.begin(), ::tolower);
            if (lowflag == "-viz") {
                std::string val = argv[i + 1];
                std::transform(val.begin(), val.end(), val.begin(), ::tolower);

                if (val == "scanline" || val == "line")
                    mode = VizMode::SCANLINE;
                else if (val == "tiles" || val == "tile")
                    mode = VizMode::TILES;
                else if (val == "random")
                    mode = VizMode::RANDOM;
                else {
                    std::cerr << "Warning: unknown viz mode, options are scanline/tiles/random not :'" << val
                              << "', defaulting to TILES\n";
                    mode = VizMode::TILES;
                }
                break;
            }
        }
        return mode;
    }
} // namespace Viz
