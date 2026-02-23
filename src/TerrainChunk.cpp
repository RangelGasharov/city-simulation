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

    for (int z = 0; z <= size; z++)
    {
        for (int x = 0; x <= size; x++)
        {
            double worldX = (double)(startX + x) * scale;
            double worldZ = (double)(startZ + z) * scale;

            Vertex v;
            float h = (float)parent->getHeight(worldX, worldZ);

            v.position = glm::vec3((float)x * scale, h, (float)z * scale);

            v.color = parent->biomeManager.getBiomeAt(worldX, worldZ).color;
            v.texUV = glm::vec2((float)x / (float)size, (float)z / (float)size);
            v.normal = glm::vec3(0.0f, 0.0f, 0.0f);

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

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        Vertex &v0 = vertices[indices[i]];
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
    {
        double wX = (double)worldPos.x + v.position.x;
        double wZ = (double)worldPos.z + v.position.z;

        float delta = 0.1f;
        float hL = (float)parent->getHeight(wX - delta, wZ);
        float hR = (float)parent->getHeight(wX + delta, wZ);
        float hD = (float)parent->getHeight(wX, wZ - delta);
        float hU = (float)parent->getHeight(wX, wZ + delta);

        v.normal = glm::normalize(glm::vec3(hL - hR, 2.0f * delta, hD - hU));
    }

    return new Mesh(vertices, indices, textures);
}