#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <deque>
#include <vector>
#include "../include/glm/glm.hpp"
#include "Perlin.h"
#include "BiomeManager.h"
class TerrainChunk;
class Shader;
class Camera;

class Terrain
{
private:
    std::deque<TerrainChunk *> generationQueue;

public:
    BiomeManager biomeManager;
    Perlin perlin;
    std::vector<TerrainChunk *> chunks;
    float worldScale = 1.0f;

    void update();
    double getHeight(double worldX, double worldZ);
    void Draw(Shader &shader, Camera &camera);
    Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed);
    ~Terrain();
};

#endif