#include <vector>
#include "headers/VBO.h"
#include "headers/Terrain.h"
#include "headers/TerrainChunk.h"
#include "headers/Mesh.h"

TerrainChunk::TerrainChunk(int x, int z, int chunkSize, int face, Terrain *parent)
{
    this->gridX = x;
    this->gridZ = z;
    this->size = (float)chunkSize;
    this->faceIndex = face;
    this->mesh = nullptr;

    glm::vec3 cubePos = getLocalCubePos(x + chunkSize / 2, z + chunkSize / 2, parent->worldScale, parent->planetRadius);
    this->worldPos = glm::normalize(cubePos) * parent->planetRadius;
}

TerrainChunk::~TerrainChunk()
{
    if (mesh)
        delete mesh;
}

void TerrainChunk::generate(Terrain *parent)
{
    if (isGenerated)
        return;
    this->mesh = generateChunkMesh(parent);
    this->isGenerated = true;
}

void TerrainChunk::Draw(Shader &shader, Camera &camera)
{
    if (!mesh)
        return;
    glm::mat4 model = glm::mat4(1.0f);
    mesh->Draw(shader, camera, model);
}

glm::vec3 TerrainChunk::getLocalCubePos(int x, int z, float worldScale, float planetRadius)
{
    double totalSideLength = (double)planetRadius * 2.0;
    double normalizedX = (double)x * (double)worldScale / totalSideLength;
    double normalizedZ = (double)z * (double)worldScale / totalSideLength;

    double u = normalizedX * 2.0 - 1.0;
    double v = normalizedZ * 2.0 - 1.0;

    u = std::max(-1.0, std::min(1.0, u));
    v = std::max(-1.0, std::min(1.0, v));

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

    double x2 = p.x * p.x;
    double y2 = p.y * p.y;
    double z2 = p.z * p.z;
    glm::dvec3 s;
    s.x = p.x * std::sqrt(std::max(0.0, 1.0 - y2 / 2.0 - z2 / 2.0 + (y2 * z2) / 3.0));
    s.y = p.y * std::sqrt(std::max(0.0, 1.0 - z2 / 2.0 - x2 / 2.0 + (z2 * x2) / 3.0));
    s.z = p.z * std::sqrt(std::max(0.0, 1.0 - x2 / 2.0 - y2 / 2.0 + (x2 * y2) / 3.0));

    return glm::vec3(s * (double)planetRadius);
}

Mesh *TerrainChunk::generateChunkMesh(Terrain *parent)
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;

    int cacheSize = size + 3;
    std::vector<float> heightCache(cacheSize * cacheSize);

    for (int z = -1; z <= (int)size + 1; z++)
    {
        for (int x = -1; x <= (int)size + 1; x++)
        {
            glm::vec3 cubePos = getLocalCubePos(gridX + x, gridZ + z, parent->worldScale, parent->planetRadius);
            float len = glm::length(cubePos);
            if (len < 0.0001f)
                cubePos = glm::vec3(0, 1, 0);

            glm::dvec3 sphereDirD = glm::normalize(glm::dvec3(cubePos));
            heightCache[(z + 1) * cacheSize + (x + 1)] = (float)parent->getHeight(sphereDirD);
        }
    }

    for (int z = 0; z <= (int)size; z++)
    {
        for (int x = 0; x <= (int)size; x++)
        {
            Vertex v;
            float h = heightCache[(z + 1) * cacheSize + (x + 1)];

            glm::vec3 cubePos = getLocalCubePos(gridX + x, gridZ + z, parent->worldScale, parent->planetRadius);

            glm::dvec3 sphereDirD = glm::normalize(glm::dvec3(cubePos));

            v.position = glm::vec3(sphereDirD * (double)(parent->planetRadius + h));
            v.normal = glm::vec3(sphereDirD);

            v.color = parent->biomeManager.getBiomeAt(glm::vec3(sphereDirD)).color;
            v.texUV = glm::vec2((float)x / size, (float)z / size);

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
            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x);

            indices.push_back(row1 + x + 1);
            indices.push_back(row2 + x + 1);
            indices.push_back(row2 + x);
        }
    }

    return new Mesh(vertices, indices, textures);
}