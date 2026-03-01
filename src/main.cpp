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
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSetScrollCallback(window, Camera::ScrollCallback);

    int seed = 6575122;
    float planetRadius = 1000000.0f;
    Terrain planet(planetRadius, seed);

    Camera camera(width, height, glm::vec3(0.0f, 0.0f, planetRadius * 3.0f));
    glfwSetWindowUserPointer(window, &camera);

    // planet.biomeManager.exportWorldBiomeMap(2048, 1024);

    double prevTime = glfwGetTime();
    unsigned int counter = 0;

    while (!glfwWindowShouldClose(window))
    {
        double crntTime = glfwGetTime();
        double timeDiff = crntTime - prevTime;
        counter++;

        if (timeDiff >= 0.1)
        {
            std::string FPS = std::to_string((1.0 / timeDiff) * counter);
            std::string ms = std::to_string((timeDiff / counter) * 1000);
            std::string newTitle = "Planet Simulation - " + FPS + " FPS / " + ms + " ms";
            glfwSetWindowTitle(window, newTitle.c_str());

            prevTime = crntTime;
            counter = 0;
        }

        static double lastFrame = 0.0;
        double currentFrame = glfwGetTime();
        float deltaTime = (float)(currentFrame - lastFrame);
        lastFrame = currentFrame;

        camera.Inputs(window, deltaTime);
        float near = 2.0f;
        float far = 10000000.0f;
        camera.updateMatrix(45.0f, near, far);

        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();
        glUniform1f(glGetUniformLocation(shaderProgram.ID, "far"), far);
        planet.update(camera.Position);
        planet.Draw(shaderProgram, camera);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    grassProgram.Delete();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}