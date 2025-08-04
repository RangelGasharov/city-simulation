#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shaderClass.h"

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    float speed = 0.1f;
    float sensitivity = 100.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;
    float FOV = 45.0f;

    bool firstClick = true;
    int width, height;

    Camera(int width, int height, glm::vec3 position);

    void Matrix(float nearPlane, float farPlane, Shader &shader, const char *uniform);
    void Inputs(GLFWwindow *window, float deltaTime);
    static void ScrollCallback(GLFWwindow *window, double xOffset, double yOffset);
    void updateOrientationFromAngles();
};
#endif