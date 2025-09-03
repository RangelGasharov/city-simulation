#include "headers/Model.h"
#include "headers/Perlin.h"
#include "headers/Terrain.h"

const unsigned int width = 1600;
const unsigned int height = 900;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    Camera *cam = reinterpret_cast<Camera *>(glfwGetWindowUserPointer(window));
    if (cam)
    {
        cam->width = width;
        cam->height = height;
    }
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

    GLFWwindow *window = glfwCreateWindow(width, height, "Test", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGL();
    glViewport(0, 0, width, height);

    Shader shaderProgram(SHADER_DIR "/default.vert", SHADER_DIR "/default.frag");
    Shader grassProgram(SHADER_DIR "/default.vert", SHADER_DIR "/grass.frag");

    glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);

    shaderProgram.Activate();
    glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
    glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_FRAMEBUFFER_SRGB);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    glfwSetWindowUserPointer(window, &camera);
    glfwSetScrollCallback(window, Camera::ScrollCallback);

    Terrain terrain(200, 200, 10.0f, 42);

    double prevTime = 0.0;
    double crntTime = 0.0;
    double timeDiff;
    unsigned int counter = 0;

    while (!glfwWindowShouldClose(window))
    {
        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;

        processInput(window);
        glClearColor(0.85f, 0.85f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        crntTime = glfwGetTime();
        timeDiff = crntTime - prevTime;
        counter++;

        camera.Inputs(window, timeDiff);

        if (timeDiff >= 1.0 / 30.0)
        {
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "YoutubeOpenGL - " + FPS + "FPS / " + ms + "ms";
            glfwSetWindowTitle(window, newTitle.c_str());

            prevTime = crntTime;
            counter = 0;
        }

        camera.updateMatrix(camera.FOV, 0.1f, 1000.0f);
        terrain.mesh->Draw(shaderProgram, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    grassProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}