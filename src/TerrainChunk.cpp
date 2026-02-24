#include <vector>
#include "headers/VBO.h"
#include "headers/Terrain.h"
#include "headers/TerrainChunk.h"
#include "headers/Mesh.h"

TerrainChunk::TerrainChunk(int x, int z, int chunkSize, float worldScale, Terrain *parent)
{
    this->gridX = x;
    this->gridZ = z;
    this->size = (float)chunkSize;
    this->worldPos = glm::vec3((float)x * worldScale, 0.0f, (float)z * worldScale);
}

void TerrainChunk::generate(float scale, Terrain *parent)
{
    if (isGenerated)
        return;

    this->mesh = generateChunkMesh(this->gridX, this->gridZ, (int)size, scale, parent);
    this->isGenerated = true;
}

void TerrainChunk::Draw(Shader &shader, Camera &camera)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPos);
    mesh->Draw(shader, camera, model);
}

Mesh *TerrainChunk::generateChunkMesh(int startX, int startZ, int size, float scale, Terrain *parent)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    int cacheSize = size + 3;
    std::vector<float> heightCache(cacheSize * cacheSize);

    for (int z = -1; z <= size + 1; z++)
    {
        for (int x = -1; x <= size + 1; x++)
        {
            double worldX = (double)(startX + x) * scale;
            double worldZ = (double)(startZ + z) * scale;
            heightCache[(z + 1) * cacheSize + (x + 1)] = (float)parent->getHeight(worldX, worldZ);
        }
    }

    for (int z = 0; z <= size; z++)
    {
        for (int x = 0; x <= size; x++)
        {
            Vertex v;
            float h = heightCache[(z + 1) * cacheSize + (x + 1)];
            v.position = glm::vec3((float)x * scale, h, (float)z * scale);

            float hL = heightCache[(z + 1) * cacheSize + x];
            float hR = heightCache[(z + 1) * cacheSize + (x + 2)];
            float hD = heightCache[z * cacheSize + (x + 1)];
            float hU = heightCache[(z + 2) * cacheSize + (x + 1)];
            v.normal = glm::normalize(glm::vec3(hL - hR, 2.0f * scale, hD - hU));

            double worldX = (double)(startX + x) * scale;
            double worldZ = (double)(startZ + z) * scale;
            v.color = parent->biomeManager.getBiomeAt(worldX, worldZ).color;
            v.texUV = glm::vec2((float)x / (float)size, (float)z / (float)size);

            vertices.push_back(v);
        }
    }

    for (int z = 0; z < size; z++)
    {
        for (int x = 0; x < size; x++)
        {
            int row1 = z * (size + 1);
            int row2 = (z + 1) * (size + 1);
            indices.push_back(row1 + x);
            indices.push_back(row2 + x);
            indices.push_back(row1 + x + 1);
            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);
            indices.push_back(row2 + x + 1);
        }
    }

    return new Mesh(vertices, indices, textures);
}