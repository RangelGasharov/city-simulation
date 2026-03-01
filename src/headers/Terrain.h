#ifndef TERRAIN_CLASS_H
#define TERRAIN_CLASS_H

#include <vector>
#include "../include/glm/glm.hpp"
#include "Perlin.h"
#include "BiomeManager.h"

class TerrainChunk;
class Shader;
class Camera;

class Terrain
{
public:
    BiomeManager biomeManager;
    Perlin perlin;

    float planetRadius;
    TerrainChunk *faces[6];

    Terrain(float radius, unsigned int seed);
    ~Terrain();

    void update(glm::vec3 cameraPos);
    void Draw(Shader &shader, Camera &camera);

    double getHeight(glm::vec3 direction);
    glm::dvec3 cubeToSphere(double u, double v, int faceIndex);
};

#endif