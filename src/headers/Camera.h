#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include "../include/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"
#include "../include/glm/gtx/rotate_vector.hpp"
#include "../include/glm/gtx/vector_query.hpp"

class Shader;
class TerrainChunk;

class Camera
{
public:
    glm::dvec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 cameraMatrix = glm::mat4(1.0f);

    bool firstClick = true;
    int width, height;
    float speed = 1.0f;
    float sensitivity = 100.0f;
    double lastX = 0.0, lastY = 0.0;
    float FOV = 45.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;

    Camera(int width, int height, glm::dvec3 position);

    void updateMatrix(float FOVdeg, float nearPlane, float farPlane);
    void Matrix(Shader &shader, const char *uniform);
    void Inputs(GLFWwindow *window, float deltaTime, float planetRadius, double currentTerrainHeight);
    void updateOrientationFromAngles();
    static void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);

    bool isInFrustum(TerrainChunk *chunk);
};
#endif