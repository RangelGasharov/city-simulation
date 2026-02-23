#include "headers/Terrain.h"
#include "headers/Perlin.h"
#include "headers/BiomeManager.h"
#include "headers/TerrainChunk.h"
#include "headers/Camera.h"
#include "headers/Mesh.h"

Terrain::Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed) : perlin(seed), biomeManager(seed + 1337)
{
    int chunkSize = 32;
    float maxDim = std::max(width, depth);
    this->worldScale = worldSize / maxDim;

    for (int z = 0; z < depth; z += chunkSize)
    {
        for (int x = 0; x < width; x += chunkSize)
        {
            TerrainChunk *chunk = new TerrainChunk(x, z, chunkSize, this->worldScale, this);
            chunks.push_back(chunk);
            generationQueue.push_back(chunk);
        }
    }
}

double Terrain::getHeight(double fx, double fz)
{
    BiomeManager::ClimatePoint climate = biomeManager.getClimate(fx, fz);
    Biome biome = biomeManager.getBiomeAt(fx, fz);

    double baseElevation = 0.0;
    if (climate.continentalness < 0.0)
    {
        baseElevation = biome.baseHeight + (climate.continentalness * 40.0);
    }
    else
    {
        baseElevation = biome.baseHeight + (climate.continentalness * 20.0);
    }

    float freq = 0.005f;

    double noiseValue = perlin.fractalNoise(
        fx * freq,
        fz * freq,
        0.0,
        biome.octaves,
        biome.persistence,
        biome.lacunarity);

    double detail = noiseValue * biome.heightScale;

    double erosionFactor = 1.0 - (climate.erosion * 0.5);
    detail *= erosionFactor;

    double finalHeight = baseElevation + detail;

    if (climate.continentalness < -0.2)
    {
        if (finalHeight > -2.0)
            finalHeight = glm::mix(finalHeight, -2.0, 0.5);
    }

    if (climate.continentalness > 0.4 && climate.weirdness > 0.5)
    {
        double steepness = (climate.weirdness - 0.5) * 2.0;
        finalHeight += std::max(0.0, noiseValue) * biome.heightScale * steepness;
    }

    return finalHeight;
}

void Terrain::update()
{
    if (!generationQueue.empty())
    {
        TerrainChunk *chunk = generationQueue.front();

        chunk->generate(this->worldScale, this);
        generationQueue.pop_front();
    }
}

void Terrain::Draw(Shader &shader, Camera &camera)
{
    for (auto chunk : chunks)
    {
        if (chunk->isGenerated && camera.isInFrustum(chunk))
        {
            chunk->Draw(shader, camera);
        }
    }
}

Terrain::~Terrain()
{
    for (auto chunk : chunks)
    {
        delete chunk->mesh;
        delete chunk;
    }
    chunks.clear();
}