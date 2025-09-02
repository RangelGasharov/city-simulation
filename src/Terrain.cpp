#include "headers/Terrain.h"
#include "headers/Perlin.h"

Terrain::Terrain(int width, int depth, float scale, unsigned int seed)
{
    mesh = generateTerrain(width, depth, scale, seed);
}

Mesh *Terrain::generateTerrain(int width, int depth, float scale, unsigned int seed)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    Perlin perlin(seed);

    for (int z = 0; z < depth; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float fx = (float)x / width;
            float fz = (float)z / depth;

            float height = perlin.noise(fx * scale, fz * scale, 0.0);

            Vertex v{};
            v.position = glm::vec3(x, height * 10.0f, z);
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
            v.color = glm::vec3(0.2f, 0.5f, 0.2f);
            v.texUV = glm::vec2(fx, fz);

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

    return new Mesh(vertices, indices, textures);
}
