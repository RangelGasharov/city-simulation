#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <vector>
#include "../include/glm/glm.hpp"
#include "Mesh.h"
#include "Perlin.h"
#include "BiomeManager.h"

class Terrain
{
public:
    Mesh *mesh;
    BiomeManager biomeManager;
    Perlin perlin;
    double getHeight(double worldX, double worldZ);
    Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed);

private:
    Mesh *generateTerrain(int width, int depth, float worldSize, float heightScale, unsigned int seed);
};

#endif