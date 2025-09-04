#include "headers/Terrain.h"
#include "headers/Perlin.h"
#include "headers/BiomeManager.h"

Terrain::Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed)
{
    mesh = generateTerrain(width, depth, worldSize, heightScale, seed);
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