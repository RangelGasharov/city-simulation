#include "headers/Camera.h"
#include "headers/TerrainChunk.h"
#include "headers/shaderClass.h"
#include <bits/stdc++.h>

Camera::Camera(int width, int height, glm::dvec3 position, float planetRadius)
{
    this->width = width;
    this->height = height;
    this->Position = position;
    this->planetRadius = planetRadius;
}

void Camera::Matrix(Shader &shader, const char *uniform)
{
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(cameraMatrix));
}

glm::dvec3 GetMouseWorldDir(Camera *cam, GLFWwindow *window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float x = (2.0f * (float)mouseX) / cam->width - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / cam->height;

    glm::mat4 invVP = glm::inverse(cam->cameraMatrix);

    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::vec4 ray_world = invVP * ray_clip;
    ray_world /= ray_world.w;

    return glm::normalize(glm::dvec3(ray_world) - cam->Position);
}

void Camera::updateCloseOrientation()
{
    double currentDist = glm::length(Position);
    double altitude = currentDist - planetRadius;

    double maxOrbitAlt = planetRadius * 0.05;
    double minOrbitAlt = 50.0;

    double groundFactor = 1.0 - std::clamp((altitude - minOrbitAlt) / (maxOrbitAlt - minOrbitAlt), 0.0, 1.0);

    groundFactor = std::pow(groundFactor, 4.0);

    double pitchAngle = glm::mix(glm::radians(90.0), glm::radians(25.0), groundFactor);

    glm::dvec3 planetUp = glm::normalize(Position);

    glm::dvec3 right = glm::cross(glm::dvec3(Orientation), planetUp);
    if (glm::length(right) < 0.001)
    {
        right = glm::cross(glm::dvec3(Up), planetUp);
    }
    right = glm::normalize(right);

    glm::dvec3 forward = glm::normalize(glm::cross(planetUp, right));

    glm::dvec3 newOrientation = forward * cos(pitchAngle) - planetUp * sin(pitchAngle);
    Orientation = glm::normalize(glm::vec3(newOrientation));

    float clampedY = std::clamp(Orientation.y, -1.0f, 1.0f);
    pitch = glm::degrees(asin(clampedY));
    yaw = glm::degrees(atan2(Orientation.z, Orientation.x));
}

void Camera::Inputs(GLFWwindow *window, float deltaTime, float planetRadius, double currentTerrainHeight)
{
    double currentDist = glm::length(Position);
    double altitude = currentDist - planetRadius;

    double maxDist = planetRadius * 20.0;
    if (currentDist > maxDist)
    {
        Position = glm::normalize(Position) * maxDist;
        currentDist = maxDist;
    }

    double dynamicSpeed = std::max(0.2, (altitude * altitude) / (planetRadius * 2.0) + altitude * 0.05) * deltaTime;
    if (altitude < 1000.0)
        dynamicSpeed = std::max(0.1, altitude * 0.2) * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        dynamicSpeed *= 5.0;

    glm::dvec3 dir = glm::dvec3(0.0);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        dir += glm::dvec3(Orientation);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        dir -= glm::dvec3(Orientation);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        dir -= glm::normalize(glm::cross(glm::dvec3(Orientation), glm::dvec3(Up)));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        dir += glm::normalize(glm::cross(glm::dvec3(Orientation), glm::dvec3(Up)));

    if (glm::length(dir) > 0.0)
    {
        Position += dir * dynamicSpeed;
        updateCloseOrientation();
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!isOrbiting)
        {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            isOrbiting = true;
        }

        double deltaX = mouseX - lastMouseX;
        double deltaY = mouseY - lastMouseY;

        double currentDist = glm::length(Position);
        double altitude = currentDist - planetRadius;

        double heightFactor = std::clamp(altitude / planetRadius, 0.001, 1.0);
        double orbitSensitivity = 0.005 * heightFactor;

        if (deltaX != 0 || deltaY != 0)
        {
            glm::dmat4 rotX = glm::rotate(glm::dmat4(1.0), -deltaX * orbitSensitivity, glm::dvec3(Up));
            Position = glm::dvec3(rotX * glm::dvec4(Position, 1.0));

            glm::dvec3 right = glm::normalize(glm::cross(glm::dvec3(Orientation), glm::dvec3(Up)));
            glm::dmat4 rotY = glm::rotate(glm::dmat4(1.0), -deltaY * orbitSensitivity, right);
            Position = glm::dvec3(rotY * glm::dvec4(Position, 1.0));

            updateCloseOrientation();
        }

        lastMouseX = mouseX;
        lastMouseY = mouseY;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (firstClick)
        {
            lastX = mouseX;
            lastY = mouseY;
            firstClick = false;
        }
        yaw += (mouseX - lastX) * 0.1f;
        pitch -= (mouseY - lastY) * 0.1f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        updateOrientationFromAngles();
        lastX = mouseX;
        lastY = mouseY;
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        isOrbiting = false;
        firstClick = true;
    }

    applyCollision(planetRadius, currentTerrainHeight);
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

void Camera::applyCollision(double planetRadius, double terrainHeight)
{
    double minSafeDist = planetRadius + terrainHeight + 2.0;
    double currentDist = glm::length(Position);

    if (currentDist < minSafeDist)
    {
        Position = glm::normalize(Position) * minSafeDist;
    }
}

void Camera::ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (cam)
    {
        double currentDist = glm::length(cam->Position);
        double altitude = currentDist - cam->planetRadius;
        glm::dvec3 zoomDir = GetMouseWorldDir(cam, window);
        double zoomSpeed = std::max(0.5, altitude * 0.1);

        glm::dvec3 nextPos = cam->Position + zoomDir * (yOffset * zoomSpeed);
        double nextDist = glm::length(nextPos);

        if (nextDist > cam->planetRadius + 5.0 && nextDist < cam->planetRadius * 20.0)
        {
            cam->Position = nextPos;
            cam->updateCloseOrientation();
        }
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