#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include "../include/glm/glm.hpp"

class Mesh;
class Terrain;
class Shader;
class Camera;

class TerrainChunk
{

public:
    bool isGenerated = false;
    glm::vec3 worldPos;
    Mesh *mesh;
    float size;
    int gridX, gridZ;

    TerrainChunk(int x, int z, int chunkSize, float worldScale, Terrain *parent);
    void generate(float scale, Terrain *parent);
    void Draw(Shader &shader, Camera &camera);

private:
    Mesh *generateChunkMesh(int startX, int startZ, int size, float scale, Terrain *parent);
};

#endif