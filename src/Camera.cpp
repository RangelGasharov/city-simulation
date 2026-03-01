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

void Camera::Inputs(GLFWwindow *window, float deltaTime, float planetRadius, double currentTerrainHeight)
{
    double altitude = glm::length(Position) - planetRadius;
    double dynamicSpeed = std::max(2.0, altitude * 0.8) * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        dynamicSpeed *= 10.0;

    glm::dvec3 dir = glm::dvec3(0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        dir += glm::dvec3(Orientation);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        dir -= glm::dvec3(Orientation);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        dir -= glm::normalize(glm::cross(glm::dvec3(Orientation), glm::dvec3(Up)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        dir += glm::normalize(glm::cross(glm::dvec3(Orientation), glm::dvec3(Up)));

    Position += dir * dynamicSpeed;

    double minSafeDist = planetRadius + currentTerrainHeight + 2.0;
    if (glm::length(Position) < minSafeDist)
    {
        Position = glm::normalize(Position) * minSafeDist;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (firstClick)
        {
            lastX = mouseX;
            lastY = mouseY;
            firstClick = false;
        }

        yaw += (mouseX - lastX) * 0.1f;
        pitch -= (mouseY - lastY) * 0.1f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        lastX = mouseX;
        lastY = mouseY;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        Orientation = glm::normalize(front);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        firstClick = true;
    }
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
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f), Orientation, Up);
    glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);
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