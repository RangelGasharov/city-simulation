#include "headers/Terrain.h"
#include "headers/Perlin.h"
#include "headers/BiomeManager.h"
#include "headers/TerrainChunk.h"
#include "headers/Camera.h"
#include "headers/Mesh.h"

Terrain::Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed) : perlin(seed), biomeManager(seed + 1337)
{
    int chunkSize = 64;
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

    float factor = 0.01;
    double elevation = biome.baseHeight + perlin.fractalNoise(fx * factor, fz * factor, 0.0, biome.octaves, biome.persistence, biome.lacunarity) * biome.heightScale;

    if (climate.continentalness < -0.5)
    {
        elevation = -20.0 + climate.continentalness * 50.0;
    }
    else if (climate.continentalness > 0.6)
    {
        elevation *= 1.5 + biome.roughness;
        elevation = std::max(elevation, 120.0);
    }

    return elevation;
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