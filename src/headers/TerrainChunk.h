#ifndef TERRAINCHUNK_H
#define TERRAINCHUNK_H

#include <vector>
#include <future>
#include <atomic>
#include "Mesh.h"

class Terrain;

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

class TerrainChunk
{
public:
    double u, v, size;
    int faceIndex, lodLevel;
    glm::dvec3 worldPos;
    bool isSplit = false;
    TerrainChunk *children[4] = {nullptr};
    Mesh *mesh = nullptr;

    std::atomic<bool> isGenerating{false};
    std::atomic<bool> hasDataReady{false};
    MeshData *pendingData = nullptr;

    TerrainChunk(double u, double v, double size, int face, int lod, Terrain *parent);
    ~TerrainChunk();

    void update(glm::dvec3 cameraPos, Terrain *parent);
    void Draw(Shader &shader, Camera &camera);
    void generateAsync(Terrain *parent);
};
#endif