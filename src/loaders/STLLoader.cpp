/*
** Custom RayTracer
** Built from scratch by Cyrian Pittaluga, 2025
** Originally developed during an EPITECH project (B-OOP-400)
** This public version is a standalone, expanded fork — all features authored independently unless otherwise noted.
*/

#include "loaders/STLLoader.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <limits>
#include <vector>

namespace RayTracer {

static bool isBinarySTL(std::ifstream& file, const std::string& filename, uint32_t& triCount)
{
    char header[80];
    file.read(header, 80);
    if (!file) return false;

    file.read(reinterpret_cast<char*>(&triCount), sizeof(triCount));
    if (!file) {
        file.clear();
        file.seekg(0, std::ios::beg);
        return false;
    }
    auto size = std::filesystem::file_size(filename);
    uint64_t expectedSize = 80ull + 4ull + static_cast<uint64_t>(triCount) * 50ull;
    return size == expectedSize;
}

    static void loadBinary(std::ifstream& file, Mesh& mesh, uint32_t triCount)
{
    for (uint32_t i = 0; i < triCount; i++) {
        file.ignore(sizeof(float) * 3);
        Math::Point3D v[3];
        for (int i = 0; i < 3; ++i) {
            float coords[3];
            file.read(reinterpret_cast<char*>(&coords), sizeof(coords));
            v[i] = {
                -static_cast<double>(coords[0]),
                 static_cast<double>(coords[2]),
                 static_cast<double>(coords[1])
            };
        }
        file.ignore(2);
        mesh.addTriangle(Triangle(v[0], v[1], v[2]));
    }
}

static void loadAscii(std::ifstream& file, Mesh& mesh)
{
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    Math::Point3D verts[3];
    int vIdx = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (token == "vertex") {
            double x, y, z;
            iss >> x >> y >> z;
            verts[vIdx++] = { x, z, y };
            if (vIdx == 3) {
                std::swap(verts[1], verts[2]);
                mesh.addTriangle(Triangle(verts[0], verts[1], verts[2]));
                vIdx = 0;
            }
        }
    }
}

static Math::Point3D calculateCenter(const Mesh& mesh)
{
    double minX = std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double minZ = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();
    double maxZ = -std::numeric_limits<double>::infinity();

    if (mesh.triangles.empty()) return {0.0, 0.0, 0.0};
    for (const auto& tri : mesh.triangles) {
        for (const Math::Point3D* v_ptr : { &tri.v0, &tri.v1, &tri.v2 }) {
            const Math::Point3D& v = *v_ptr;
            minX = std::min(minX, v.x);
            minY = std::min(minY, v.y);
            minZ = std::min(minZ, v.z);
            maxX = std::max(maxX, v.x);
            maxY = std::max(maxY, v.y);
            maxZ = std::max(maxZ, v.z);
        }
    }
    return { (minX + maxX) * 0.5, (minY + maxY) * 0.5, (minZ + maxZ) * 0.5 };
}

static void centerMesh(Mesh& mesh)
{
    Math::Point3D center = calculateCenter(mesh);

    if (mesh.triangles.empty()) return;
    for (auto& tri : mesh.triangles) {
        tri.v0.x -= center.x; tri.v0.y -= center.y; tri.v0.z -= center.z;
        tri.v1.x -= center.x; tri.v1.y -= center.y; tri.v1.z -= center.z;
        tri.v2.x -= center.x; tri.v2.y -= center.y; tri.v2.z -= center.z;
    }
}

Mesh STLLoader::load(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open STL file: " + filename);

    Mesh mesh;
    uint32_t triCount = 0;
    if (isBinarySTL(file, filename, triCount)) {
        loadBinary(file, mesh, triCount);
    } else {
        loadAscii(file, mesh);
    }
    centerMesh(mesh);
    std::size_t count = mesh.triangles.size();
    std::cerr << "Loaded " << count << " triangles from " << filename << std::endl;

    return mesh;
}

} // namespace RayTracer
