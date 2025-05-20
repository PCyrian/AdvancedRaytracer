/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "../../include/core/Raytracer.hpp"
#include "../../include/core/Visualizer.hpp"
#include "../../include/rendering/FASTRenderer.hpp"
#include "../../include/rendering/Skybox.hpp"
#include "../../include/rendering/BDPT.hpp"

#include <vector>
#include <cmath>
#include <algorithm>
#include "core/Vector3D.hpp"

static void denoise(const std::vector<Math::Vector3D>& in,
             std::vector<Math::Vector3D>& out,
             int width, int height)
{
    std::cerr << "Starting denoising" << std::endl;
    const float sigma_spatial = 1.0f;
    const float sigma_range   = 0.08f;
    const int   radius        = 1;

    out = in;

    auto gaussian = [&](float d, float sigma){
        return std::exp(-0.5f * (d*d) / (sigma*sigma));
    };

    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            Math::Vector3D sum(0,0,0);
            float wsum = 0.0f;
            const Math::Vector3D& center = in[y * width + x];

            for(int dy = -radius; dy <= radius; ++dy) {
                for(int dx = -radius; dx <= radius; ++dx) {
                    int nx = std::clamp(x + dx, 0, width - 1);
                    int ny = std::clamp(y + dy, 0, height - 1);
                    const Math::Vector3D& sample = in[ny * width + nx];

                    float gs = gaussian(std::sqrt(float(dx*dx + dy*dy)), sigma_spatial);
                    float gr = gaussian((sample - center).length(), sigma_range);
                    float w  = gs * gr;

                    sum  += sample * w;
                    wsum += w;
                }
            }
            out[y * width + x] = (wsum > 0.0f) ? (sum / wsum) : center;
        }
    }
    std::cerr << "Done" << std::endl;
}


Raytracer::Raytracer(int ac, char **av)
{
    _ac = ac;
    _av = av;
}

void Raytracer::run() {
    if (!Utils::checkUsage(_ac, _av))
        exit(84);

    int threads = 1;

    try {
        threads = Utils::parseThreads(_ac, _av);
    } catch(const std::exception& e) {
        throw Raytracer::Error("Incorect thread argument. Negative or Null.");
    }

    bool showProgress = Utils::parseProgress(_ac, _av);
    auto params = Utils::getRenderParams(_ac, _av);
    bool debug = Utils::isDebugModeEnable(_ac, _av);
#ifdef USE_SFML
    Viz::VizMode mode = Viz::parseVizMode(_ac, _av);
#else
    [[maybe_unused]] Viz::VizMode mode = Viz::VizMode::DEFAULT;
#endif

    omp_set_num_threads(threads);

    auto cam = Utils::createCamera(params);
    RayTracer::Scene scene(cam);
    if (!Utils::loadSceneOrExit(_av[1], scene, debug))
        throw Raytracer::Error("Failed to load scene.");

    cam = scene.camera;
    params.H = cam.height;
    params.W = cam.width;

    scene.bvhRoot = Utils::buildBVH(scene.objects);

    std::vector<Math::Vector3D> framebuffer;
#ifdef USE_SFML
    if (mode != Viz::VizMode::DEFAULT) {
        framebuffer = Viz::renderImageSFML(scene, params, mode, showProgress);
    } else
#endif
    {
        if (params.fast) {
            framebuffer = BDPT::renderBDPT(scene, params, showProgress);
        } else {
            framebuffer = Utils::renderImage(scene, params, showProgress);
        }
    }
    PostFX::postProcess(
            framebuffer,
            params.W, params.H,
            /*contrast=*/2.0f,
            /*saturation=*/40.0f,
            /*vignetteStrength=*/0.0f
    );
    std::vector<Math::Vector3D> denoised;
    denoise(framebuffer, denoised, params.W, params.H);
    Utils::writePPM(denoised, params.W, params.H);
}

Raytracer::~Raytracer()
{
}
