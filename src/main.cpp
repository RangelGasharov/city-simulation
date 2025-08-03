#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <cmath>
#include "headers/shaderClass.h"
#include "headers/VAO.h"
#include "headers/VBO.h"
#include "headers/EBO.h"
#include "headers/Texture.h"
#include "stb/stb_image.h"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Test", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glClearColor(0.09f, 0.16f, 0.25f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

    unsigned int indices[] = {
        0, 2, 1,
        0, 3, 2};

    Shader shaderProgram(SHADER_DIR "/default.vert", SHADER_DIR "/default.frag");

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices, sizeof(vertices));
    EBO EBO1(indices, sizeof(indices));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 8 * sizeof(float), (void *)0);
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    GLuint uniID = glGetUniformLocation(shaderProgram.ID, "scale");

    Texture brickTexture(TEXTURE_DIR "/brick_texture.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    brickTexture.texUnit(shaderProgram, "tex0", 0);

    GLuint tex0Uni = glGetUniformLocation(shaderProgram.ID, "tex0");
    glUniform1i(tex0Uni, 0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.09f, 0.16f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        glUniform1f(uniID, 0.0f);
        brickTexture.Bind();

        VAO1.Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    brickTexture.Delete();
    shaderProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}