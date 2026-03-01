#include "headers/Terrain.h"
#include "headers/TerrainChunk.h"
#include <algorithm>

Terrain::Terrain(float radius, unsigned int seed)
    : perlin(seed),
      biomeManager(seed + 1337),
      threadPool(std::thread::hardware_concurrency()),
      planetRadius(radius),
      seed(seed)
{
    for (int i = 0; i < 6; i++)
    {
        faces[i] = new TerrainChunk(-1.0, -1.0, 2.0, i, 0, this);
    }
}

Terrain::~Terrain()
{
    for (int i = 0; i < 6; i++)
        delete faces[i];
}

void Terrain::update(glm::vec3 cameraPos)
{
    lastCameraPos = glm::dvec3(cameraPos);
    for (int i = 0; i < 6; i++)
    {
        faces[i]->update(lastCameraPos, this);
    }
}

void Terrain::Draw(Shader &shader, Camera &camera)
{
    for (int i = 0; i < 6; i++)
    {
        faces[i]->Draw(shader, camera);
    }
}

glm::dvec3 Terrain::cubeToSphere(double u, double v, int faceIndex)
{
    glm::dvec3 p;
    switch (faceIndex)
    {
    case 0:
        p = {1.0, v, -u};
        break;
    case 1:
        p = {-1.0, v, u};
        break;
    case 2:
        p = {u, 1.0, -v};
        break;
    case 3:
        p = {u, -1.0, v};
        break;
    case 4:
        p = {u, v, 1.0};
        break;
    case 5:
        p = {-u, v, -1.0};
        break;
    default:
        p = {0, 0, 0};
    }

    double x2 = p.x * p.x, y2 = p.y * p.y, z2 = p.z * p.z;
    glm::dvec3 s;
    s.x = p.x * std::sqrt(std::max(0.0, 1.0 - y2 / 2.0 - z2 / 2.0 + (y2 * z2) / 3.0));
    s.y = p.y * std::sqrt(std::max(0.0, 1.0 - z2 / 2.0 - x2 / 2.0 + (z2 * x2) / 3.0));
    s.z = p.z * std::sqrt(std::max(0.0, 1.0 - x2 / 2.0 - y2 / 2.0 + (x2 * y2) / 3.0));
    return s;
}

double Terrain::getHeight(glm::vec3 direction)
{
    glm::dvec3 p = glm::normalize(glm::dvec3(direction));
    BiomeManager::ClimatePoint climate = biomeManager.getClimate(glm::vec3(p));
    Biome biome = biomeManager.getBiomeAt(glm::vec3(p));

    double baseHeight = (climate.continentalness > 0.0)
                            ? std::pow(climate.continentalness, 0.5) * 70.0
                            : -std::pow(std::abs(climate.continentalness), 1.0) * 130.0;

    float freq = 2.5f * (1000.0f / planetRadius);
    double noiseValue = perlin.fractalNoise(p.x * freq, p.y * freq, p.z * freq, biome.octaves, biome.persistence, biome.lacunarity);

    double mountainPower = std::clamp((climate.weirdness - 0.4) / 0.4, 0.0, 1.0);
    double ridge = std::pow(1.0 - std::abs(noiseValue), 2.0);
    double detail = ridge * biome.heightScale * (0.1 + mountainPower * 0.9);

    return baseHeight + detail;
}