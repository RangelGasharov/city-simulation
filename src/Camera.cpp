#include "headers/Camera.h"

Camera::Camera(int width, int height, glm::vec3 position)
{
    Camera::width = width;
    Camera::height = height;
    Position = position;

    updateOrientationFromAngles();
}

void Camera::Matrix(float nearPlane, float farPlane, Shader &shader, const char *uniform)
{
    glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up);
    glm::mat4 projection = glm::perspective(glm::radians(FOV), (float)width / height, nearPlane, farPlane);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
}

void Camera::Inputs(GLFWwindow *window, float deltaTime)
{
    float currentSpeed = speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        currentSpeed = 1.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += currentSpeed * Orientation * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= currentSpeed * Orientation * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Orientation, Up)) * currentSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Orientation, Up)) * currentSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        Position += currentSpeed * Up * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        Position -= currentSpeed * Up * deltaTime;

    float rotationAmount = glm::radians(45.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        yaw -= glm::degrees(rotationAmount);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        yaw += glm::degrees(rotationAmount);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        if (firstClick)
        {
            glfwSetCursorPos(window, width / 2, height / 2);
            firstClick = false;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        float rotX = sensitivity * (mouseY - height / 2) / height;
        float rotY = sensitivity * (mouseX - width / 2) / width;

        pitch -= rotX;
        yaw -= rotY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glfwSetCursorPos(window, width / 2, height / 2);
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

void Camera::ScrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    Camera *cam = static_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (!cam)
        return;

    cam->FOV -= (float)yOffset * 2.0f;
    if (cam->FOV < 30.0f)
        cam->FOV = 30.0f;
    if (cam->FOV > 90.0f)
        cam->FOV = 90.0f;
}