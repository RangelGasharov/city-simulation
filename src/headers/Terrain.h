#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <vector>
#include "../include/glm/glm.hpp"
#include "Mesh.h"
#include "Perlin.h"

class Terrain
{
public:
    Mesh *mesh;

    Terrain(int width, int depth, float worldSize, float heightScale, unsigned int seed);

private:
    Mesh *generateTerrain(int width, int depth, float worldSize, float heightScale, unsigned int seed);
};

#endif