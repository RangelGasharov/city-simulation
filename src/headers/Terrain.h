#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <deque>
#include <map>
#include <vector>
#include "../include/glm/glm.hpp"
#include "Perlin.h"
#include "BiomeManager.h"

class TerrainChunk;
class Shader;
class Camera;

struct ChunkKey
{
    int face;
    int x, z;

    bool operator<(const ChunkKey &other) const
    {
        if (face != other.face)
            return face < other.face;
        if (x != other.x)
            return x < other.x;
        return z < other.z;
    }
};

class Terrain
{
private:
    glm::vec2 projectToFace(glm::vec3 p, int face);
    void cleanupChunks(glm::vec3 cameraPos);

public:
    BiomeManager biomeManager;
    Perlin perlin;

    float worldScale = 1.0f;
    float planetRadius;
    int chunkSize;
    int renderDistance;

    std::map<ChunkKey, TerrainChunk *> activeChunks;
    std::deque<TerrainChunk *> generationQueue;

    Terrain(float radius, unsigned int seed);
    ~Terrain();

    void update(glm::vec3 cameraPos);
    void updateFace(int face, glm::vec3 cameraPos);
    double getHeight(glm::vec3 direction);
    void Draw(Shader &shader, Camera &camera);
};

#endif