#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include "../include/glm/glm.hpp"
#include <vector>

class Mesh;
class Terrain;
class Shader;
class Camera;

class TerrainChunk
{
public:
    // Quadtree Daten
    double u, v, size; // Bereich im Face-Space (-1.0 bis 1.0)
    int faceIndex;
    int lodLevel;

    bool isGenerated = false;
    bool isSplit = false;

    glm::dvec3 worldPos; // Mittelpunkt in double für Präzision
    Mesh *mesh = nullptr;
    TerrainChunk *children[4] = {nullptr, nullptr, nullptr, nullptr};

    TerrainChunk(double u, double v, double size, int face, int lod, Terrain *parent);
    ~TerrainChunk();

    void update(glm::dvec3 cameraPos, Terrain *parent);
    void Draw(Shader &shader, Camera &camera);

private:
    Mesh *generateChunkMesh(Terrain *parent);
};

#endif