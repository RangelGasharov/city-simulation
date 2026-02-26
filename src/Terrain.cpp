#include "headers/Terrain.h"
#include "headers/Perlin.h"
#include "headers/BiomeManager.h"
#include "headers/TerrainChunk.h"
#include "headers/Camera.h"
#include "headers/Mesh.h"
#include <algorithm>

Terrain::Terrain(float radius, unsigned int seed) : perlin(seed), biomeManager(seed + 1337)
{
    this->planetRadius = radius;
    this->chunkSize = 32;
    this->renderDistance = 12;
    this->worldScale = 100.0f;
}

double smoothRemap(double value, double inMin, double inMax)
{
    double t = (value - inMin) / (inMax - inMin);
    t = std::clamp(t, 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

double Terrain::getHeight(glm::vec3 direction)
{
    glm::vec3 p = glm::normalize(direction);

    BiomeManager::ClimatePoint climate = biomeManager.getClimate(p);
    Biome biome = biomeManager.getBiomeAt(p);

    double baseHeight;

    if (climate.continentalness > 0.0)
    {
        baseHeight = std::pow(climate.continentalness, 0.8) * 40.0;
    }
    else
    {
        baseHeight = -std::pow(std::abs(climate.continentalness), 1.2) * 60.0;
    }

    double erosionMuliplier = std::clamp(1.0 - climate.erosion, 0.1, 1.0);

    float freq = 0.5f;

    double noiseValue = perlin.fractalNoise(
        direction.x * freq,
        direction.y * freq,
        direction.z * freq,
        biome.octaves,
        biome.persistence,
        biome.lacunarity);

    double detail = noiseValue * biome.heightScale * erosionMuliplier;

    double mountainMask = std::clamp((climate.weirdness - 0.5) / 0.3, 0.0, 1.0);
    double peaks = std::abs(noiseValue) * mountainMask * 80.0;

    double finalHeight = baseHeight + detail + peaks;

    return finalHeight;
}

glm::vec2 Terrain::projectToFace(glm::vec3 p, int face)
{
    float x = p.x, y = p.y, z = p.z;
    float absX = std::abs(x), absY = std::abs(y), absZ = std::abs(z);

    float u = 0, v = 0;
    if (face == 0)
    {
        u = -z / absX;
        v = y / absX;
    }
    else if (face == 1)
    {
        u = z / absX;
        v = y / absX;
    }
    else if (face == 2)
    {
        u = x / absY;
        v = -z / absY;
    }
    else if (face == 3)
    {
        u = x / absY;
        v = z / absY;
    }
    else if (face == 4)
    {
        u = x / absZ;
        v = y / absZ;
    }
    else if (face == 5)
    {
        u = -x / absZ;
        v = y / absZ;
    }

    return glm::vec2(u, v);
}

void Terrain::updateFace(int face, glm::vec3 cameraPos)
{
    glm::vec3 dir = glm::normalize(cameraPos);
    float sideLength = planetRadius * 2.0f;
    glm::vec2 faceCoords = projectToFace(dir, face) * (sideLength / 2.0f);

    int centerChunkX = static_cast<int>(std::floor(faceCoords.x / (chunkSize * worldScale)));
    int centerChunkZ = static_cast<int>(std::floor(faceCoords.y / (chunkSize * worldScale)));

    glm::vec3 faceNormals[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    if (glm::dot(dir, faceNormals[face]) < -0.2f)
        return;

    for (int z = -renderDistance; z <= renderDistance; z++)
    {
        for (int x = -renderDistance; x <= renderDistance; x++)
        {
            int gridX = (centerChunkX + x) * chunkSize;
            int gridZ = (centerChunkZ + z) * chunkSize;
            ChunkKey key = {face, gridX, gridZ};

            if (activeChunks.find(key) == activeChunks.end())
            {
                TerrainChunk *newChunk = new TerrainChunk(gridX, gridZ, chunkSize, face, this);
                activeChunks[key] = newChunk;
                generationQueue.push_back(newChunk);
            }
        }
    }
}

void Terrain::cleanupChunks(glm::vec3 cameraPos)
{
    double maxDist = (double)(renderDistance + 4) * chunkSize * worldScale;
    glm::dvec3 camPosD = glm::dvec3(cameraPos);

    for (auto it = activeChunks.begin(); it != activeChunks.end();)
    {
        glm::dvec3 chunkPosD = glm::dvec3(it->second->worldPos);

        if (glm::distance(camPosD, chunkPosD) > maxDist)
        {
            TerrainChunk *chunk = it->second;

            auto qIt = std::find(generationQueue.begin(), generationQueue.end(), chunk);
            if (qIt != generationQueue.end())
                generationQueue.erase(qIt);

            delete chunk->mesh;
            delete chunk;
            it = activeChunks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Terrain::update(glm::vec3 cameraPos)
{
    for (int face = 0; face < 6; face++)
    {
        updateFace(face, cameraPos);
    }

    cleanupChunks(cameraPos);

    if (!generationQueue.empty())
    {
        TerrainChunk *chunk = generationQueue.front();
        generationQueue.pop_front();

        ChunkKey key = {chunk->faceIndex, chunk->gridX, chunk->gridZ};
        if (activeChunks.count(key))
        {
            chunk->generate(this);
        }
    }
}

void Terrain::Draw(Shader &shader, Camera &camera)
{
    for (auto const &[key, chunk] : activeChunks)
    {
        if (chunk->isGenerated)
        {
            chunk->Draw(shader, camera);
        }
    }
}

Terrain::~Terrain()
{
    for (auto const &[key, chunk] : activeChunks)
    {
        delete chunk;
    }
    activeChunks.clear();
    generationQueue.clear();
}