#include "headers/Terrain.h"
#include "headers/Perlin.h"
#include "headers/BiomeManager.h"

Terrain::Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed) : perlin(seed), biomeManager(seed + 1337)
{
    mesh = generateTerrain(width, depth, worldSize, heightScale, seed);
}

double Terrain::getHeight(double fx, double fz)
{
    BiomeManager::ClimatePoint climate = biomeManager.getClimate(fx, fz);
    Biome biome = biomeManager.getBiomeAt(fx, fz);

    double elevation = biome.baseHeight + perlin.fractalNoise(fx, fz, 0.0, biome.octaves, biome.persistence, biome.lacunarity) * biome.heightScale;

    if (climate.continentalness < -0.5)
    {
        elevation = -20.0 + climate.continentalness * 50.0;
    }
    else if (climate.continentalness > 0.6)
    {
        elevation *= 1.5 + biome.roughness;
        elevation = std::max(elevation, 120.0);
    }

    double ridged = 1.0 - fabs(perlin.noise(fx * 0.00015, fz * 0.00015, 0.0));
    if (ridged > 0.98)
    {
        double depth = (ridged - 0.97) * 300.0;
        elevation -= depth;

        if (elevation < 0.0)
            elevation = -5.0;
    }

    double riverNoise = fabs(perlin.noise(fx * 0.0005, fz * 0.0005, 0.0));
    if (riverNoise < 0.02)
    {
        double riverDepth = (0.02 - riverNoise) * 200.0;
        elevation -= riverDepth;
        if (elevation < 0.0)
            elevation = -5.0;
    }

    if (elevation > 50.0)
    {
        double detail = perlin.noise(fx * 0.01, fz * 0.01, 2000.0);
        elevation += detail * (biome.roughness * 20.0);
    }

    if (elevation < 2.0 && climate.continentalness > -0.2)
    {
        elevation = -2.0;
    }

    return elevation;
}

Mesh *Terrain::generateTerrain(int width, int depth, float worldSize, float heightScale, unsigned int seed)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    Perlin perlin(seed);
    BiomeManager biomeManager(seed + 1337);

    int octaves = 16;
    double persistence = 0.45;
    double lacunarity = 2.0;

    float halfW = (float)width / 2.0f;
    float halfD = (float)depth / 2.0f;
    float maxDim = std::max(width, depth);

    biomeManager.exportWorldBiomeMap("terrain-biomes.ppm", 2048, 2048, 20000, 20000);

    for (int z = 0; z < depth; z++)
    {
        for (int x = 0; x < width; x++)
        {
            double fx = (double)x / maxDim * worldSize;
            double fz = (double)z / maxDim * worldSize;

            Biome biome = biomeManager.getBiomeAt(x, z);
            double height = biome.baseHeight + perlin.fractalNoise(fx, fz, 0.0, biome.octaves, biome.persistence, biome.lacunarity) * biome.heightScale;

            Vertex v{};
            v.position = glm::vec3((float)x - halfW, height, (float)z - halfD);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            v.color = biome.color;
            v.texUV = glm::vec2((float)x / (float)(width - 1), (float)z / (float)(depth - 1));

            vertices.push_back(v);
        }
    }

    for (int z = 0; z < depth - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    for (auto &v : vertices)
        v.normal = glm::vec3(0.0f);

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        Vertex &v0 = vertices[indices[i + 0]];
        Vertex &v1 = vertices[indices[i + 1]];
        Vertex &v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        v0.normal += faceNormal;
        v1.normal += faceNormal;
        v2.normal += faceNormal;
    }

    for (auto &v : vertices)
        v.normal = glm::normalize(v.normal);

    return new Mesh(vertices, indices, textures);
}