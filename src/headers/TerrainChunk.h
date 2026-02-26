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
    int faceIndex;

    TerrainChunk(int x, int z, int chunkSize, int face, Terrain *parent);
    ~TerrainChunk();

    void generate(Terrain *parent);
    void Draw(Shader &shader, Camera &camera);

private:
    glm::vec3 getLocalCubePos(int x, int z, float worldScale, float planetRadius);
    Mesh *generateChunkMesh(Terrain *parent);
};

#endif