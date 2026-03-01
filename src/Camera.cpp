#include "headers/Camera.h"
#include "headers/TerrainChunk.h"
#include "headers/shaderClass.h"

Camera::Camera(int width, int height, glm::dvec3 position)
{
    this->width = width;
    this->height = height;
    this->Position = position;
}

void Camera::Matrix(Shader &shader, const char *uniform)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

void Camera::Inputs(GLFWwindow *window, float deltaTime)
{
    float distFromCenter = (float)glm::length(Position);
    float currentSpeed = (distFromCenter * 0.5f + speed) * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed *= 10.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += glm::dvec3(Orientation) * (double)currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= glm::dvec3(Orientation) * (double)currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::dvec3(glm::normalize(glm::cross(Orientation, Up))) * (double)currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::dvec3(glm::normalize(glm::cross(Orientation, Up))) * (double)currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position += glm::dvec3(Up) * (double)currentSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position -= glm::dvec3(Up) * (double)currentSpeed;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        if (firstClick)
        {
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotX = sensitivity * (float)(mouseY - height / 2.0) / height;
        float rotY = sensitivity * (float)(mouseX - width / 2.0) / width;

        pitch -= rotX;
        yaw += rotY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glfwSetCursorPos(window, width / 2.0, height / 2.0);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }

    updateOrientationFromAngles();
}

void Camera::updateOrientationFromAngles()
{
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    Orientation = glm::normalize(direction);
}

void Camera::updateMatrix(float FOVdeg, float nearPlane, float farPlane)
{
    glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f), Orientation, Up);

    cameraMatrix = projection * view;
}

void Camera::ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (cam)
    {
        cam->FOV -= (float)yOffset * 2.0f;
        cam->FOV = glm::clamp(cam->FOV, 10.0f, 120.0f);
    }
}

bool Camera::isInFrustum(TerrainChunk *chunk)
{
    glm::dvec3 chunkCenter = chunk->worldPos;
    double dist = glm::distance(this->Position, chunkCenter);

    if (dist > (double)chunk->size * 500000.0)
        return false;

    glm::dvec3 toChunk = glm::normalize(chunkCenter - this->Position);
    double dot = glm::dot(glm::dvec3(this->Orientation), toChunk);

    return dot > -0.2;
}