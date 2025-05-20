/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include <omp.h>
#include "../../include/core/Raytracer.hpp"

int main(int argc, char* argv[])
{
    Raytracer raimond(argc, argv);

    try {
        raimond.run();
        return 0;
    }
    catch(const Raytracer::Error &error)
    {
        std::cerr << "[ERROR] - " << error.what() << std::endl;
        return 84;
    }    
}
