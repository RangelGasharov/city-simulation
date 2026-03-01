#include "headers/TerrainChunk.h"
#include "headers/Terrain.h"

TerrainChunk::TerrainChunk(double u, double v, double size, int face, int lod, Terrain *parent)
    : u(u), v(v), size(size), faceIndex(face), lodLevel(lod)
{
    glm::dvec3 sphereDir = parent->cubeToSphere(u + size / 2.0, v + size / 2.0, faceIndex);
    this->worldPos = sphereDir * (double)parent->planetRadius;
}

TerrainChunk::~TerrainChunk()
{
    if (isSplit)
    {
        for (int i = 0; i < 4; i++)
            delete children[i];
    }
    delete mesh;
    if (pendingData)
        delete pendingData;
}

void TerrainChunk::update(glm::dvec3 cameraPos, Terrain *parent)
{
    double distance = glm::distance(cameraPos, worldPos);

    bool shouldSplit = (distance < size * parent->planetRadius * 3.0) && (lodLevel < 18);

    if (shouldSplit)
    {
        if (!isSplit)
        {
            double half = size / 2.0;
            children[0] = new TerrainChunk(u, v, half, faceIndex, lodLevel + 1, parent);
            children[1] = new TerrainChunk(u + half, v, half, faceIndex, lodLevel + 1, parent);
            children[2] = new TerrainChunk(u, v + half, half, faceIndex, lodLevel + 1, parent);
            children[3] = new TerrainChunk(u + half, v + half, half, faceIndex, lodLevel + 1, parent);
            isSplit = true;
        }

        bool allChildrenReady = true;
        for (int i = 0; i < 4; i++)
        {
            children[i]->update(cameraPos, parent);
            if (!children[i]->mesh && !children[i]->isSplit)
                allChildrenReady = false;
        }

        if (allChildrenReady && mesh)
        {
            delete mesh;
            mesh = nullptr;
        }
    }
    else
    {
        if (isSplit)
        {
            for (int i = 0; i < 4; i++)
            {
                delete children[i];
                children[i] = nullptr;
            }
            isSplit = false;
        }

        if (!mesh && !isGenerating && !hasDataReady)
        {
            generateAsync(parent);
        }

        if (hasDataReady && pendingData)
        {
            if (mesh)
                delete mesh;
            mesh = new Mesh(pendingData->vertices, pendingData->indices);
            delete pendingData;
            pendingData = nullptr;
            hasDataReady = false;
        }
    }
}

void TerrainChunk::generateAsync(Terrain *parent)
{
    isGenerating = true;
    glm::dvec3 camPos = parent->lastCameraPos;
    double dist = glm::distance(camPos, worldPos);

    int priority = (int)(1000000.0 - dist);

    parent->threadPool.enqueue(priority, [this, parent]()
                               {
        MeshData* data = new MeshData();
        
        const int res = (lodLevel < 5) ? 16 : (lodLevel < 10 ? 32 : 64);
        double step = size / (double)res;

        for (int z = 0; z <= res; z++) {
            for (int x = 0; x <= res; x++) {
                double currU = u + (double)x * step;
                double currV = v + (double)z * step;
                glm::dvec3 sphereDir = parent->cubeToSphere(currU, currV, faceIndex);
                double h = parent->getHeight(glm::vec3(sphereDir));
                glm::dvec3 absPos = sphereDir * (double)(parent->planetRadius + h);

                Vertex vtx;
                vtx.position = glm::vec3(absPos - worldPos); 
                vtx.normal = glm::vec3(sphereDir);
                vtx.color = parent->biomeManager.getBiomeAt(glm::vec3(sphereDir)).color;
                vtx.texUV = glm::vec2((float)x / (float)res, (float)z / (float)res);
                data->vertices.push_back(vtx);
            }
        }

        for (int z = 0; z < res; z++) {
            for (int x = 0; x < res; x++) {
                int r1 = z * (res + 1);
                int r2 = (z + 1) * (res + 1);
                data->indices.push_back(r1 + x);
                data->indices.push_back(r1 + x + 1);
                data->indices.push_back(r2 + x);
                data->indices.push_back(r1 + x + 1);
                data->indices.push_back(r2 + x + 1);
                data->indices.push_back(r2 + x);
            }
        }

        this->pendingData = data;
        this->isGenerating = false;
        this->hasDataReady = true; });
}

void TerrainChunk::Draw(Shader &shader, Camera &camera)
{
    if (isSplit)
    {
        bool childrenRenderable = true;
        for (int i = 0; i < 4; i++)
        {
            if (!children[i]->mesh && !children[i]->isSplit)
            {
                childrenRenderable = false;
                break;
            }
        }

        if (childrenRenderable)
        {
            for (int i = 0; i < 4; i++)
                children[i]->Draw(shader, camera);
        }
        else if (mesh)
        {
            glm::vec3 relPos = glm::vec3(worldPos - camera.Position);
            mesh->Draw(shader, camera, glm::mat4(1.0f), relPos);
        }
    }
    else if (mesh)
    {
        glm::vec3 relPos = glm::vec3(worldPos - camera.Position);
        mesh->Draw(shader, camera, glm::mat4(1.0f), relPos);
    }
}