#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "loadTexture.h"

// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

const unsigned int SCREEN_WIDTH = 1300;
const unsigned int SCREEN_HEIGHT = 850;

Camera camera;
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Test 4", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Captures mouse

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader lightingShader("../resources/shaders/block.vert", "../resources/shaders/block.frag");
    Shader lightCubeShader("../resources/shaders/light_cube.vert", "../resources/shaders/light_cube.frag");

    float vertices[] = {
         // positions         // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // glm::vec3 pointLightPositions[] = {
    //     glm::vec3( 0.7f,  0.2f,  2.0f),
    //     glm::vec3( 2.3f, -3.3f, -4.0f),
    //     glm::vec3(-4.0f,  2.0f, -12.0f),
    //     glm::vec3( 0.0f,  0.0f, -3.0f)
    // };

    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);                   // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); // texture coordinates
    glEnableVertexAttribArray(2);

    // configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ground mesh
    std::vector<Vertex> groundVertices;
    //                        position                            normal                       texture coordinates
    groundVertices.push_back({glm::vec3(-100.0f, -4.0f, -100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 100.0f)});
    groundVertices.push_back({glm::vec3( 100.0f, -4.0f, -100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(100.0f, 100.0f)});
    groundVertices.push_back({glm::vec3( 100.0f, -4.0f,  100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(100.0f, 0.0f)});
    groundVertices.push_back({glm::vec3(-100.0f, -4.0f,  100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)});
    std::vector<unsigned int> groundIndices{0, 1, 2, 0, 2, 3};
    std::vector<Texture> groundTextures{
        {loadTexture(std::string("../resources/textures/grid.png").c_str(), []() {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }), "texture_diffuse"}
        // {loadTexture(std::string("../resources/textures/grid.png").c_str()), "texture_specular"}
    };
    glm::vec3 groundColor(0.0f, 0.0f, 1.0f);
    Mesh ground(groundVertices, groundIndices, groundTextures, groundColor);

    // Oskar donut
    Model model(std::string("../resources/3d-models/donut_oskar/donut_oskar.obj").c_str());

    unsigned int diffuseMap = loadTexture(std::string("../resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(std::string("../resources/textures/container2_specular.png").c_str());

    lightingShader.use();
    // lightingShader.setInt("uMaterial.diffuse", 0);
    // lightingShader.setInt("uMaterial.specular", 1);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightingShader.use();
        lightingShader.setFloat("uMaterial.shininess", 32.0f);

        lightingShader.setBool("uDoSunlight", true);
        lightingShader.setVec3("uSunlight.ambient", glm::vec3(0.5f));
        lightingShader.setVec3("uSunlight.diffuse", glm::vec3(1.4f));
        lightingShader.setVec3("uSunlight.specular", glm::vec3(0.6f));

        lightingShader.setBool("uNoTextures", false);

        // point light 1
        // lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        // lightingShader.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        // lightingShader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        // lightingShader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        // lightingShader.setFloat("pointLights[0].constant", 1.0f);
        // lightingShader.setFloat("pointLights[0].linear", 0.09f);
        // lightingShader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        // lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        // lightingShader.setVec3("pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        // lightingShader.setVec3("pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        // lightingShader.setVec3("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        // lightingShader.setFloat("pointLights[1].constant", 1.0f);
        // lightingShader.setFloat("pointLights[1].linear", 0.09f);
        // lightingShader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        // lightingShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        // lightingShader.setVec3("pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        // lightingShader.setVec3("pointLights[2].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        // lightingShader.setVec3("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        // lightingShader.setFloat("pointLights[2].constant", 1.0f);
        // lightingShader.setFloat("pointLights[2].linear", 0.09f);
        // lightingShader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        // lightingShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        // lightingShader.setVec3("pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        // lightingShader.setVec3("pointLights[3].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        // lightingShader.setVec3("pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        // lightingShader.setFloat("pointLights[3].constant", 1.0f);
        // lightingShader.setFloat("pointLights[3].linear", 0.09f);
        // lightingShader.setFloat("pointLights[3].quadratic", 0.032f);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = camera.getViewMatrix();
        lightingShader.setMat4("uProjection", glm::value_ptr(projection));
        lightingShader.setMat4("uView", glm::value_ptr(view));
        lightingShader.setVec3("uSunlight.direction", glm::mat3(view) * glm::normalize(glm::vec3(0.5f, -1.0f, -0.3f)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        glBindVertexArray(cubeVAO);
        for (unsigned int i = 0; i < 10; i++) {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("uModel", glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        lightingShader.setVec3("uSunlight.ambient", glm::vec3(0.5f));
        lightingShader.setVec3("uSunlight.diffuse", glm::vec3(1.2f));
        lightingShader.setVec3("uSunlight.specular", glm::vec3(0.4f));
        glm::mat4 groundModel(1.0f);
        lightingShader.setMat4("uModel", glm::value_ptr(groundModel));
        ground.draw(lightingShader);
        lightingShader.setVec3("uSunlight.ambient", glm::vec3(0.5f));
        lightingShader.setVec3("uSunlight.diffuse", glm::vec3(1.4f));
        lightingShader.setVec3("uSunlight.specular", glm::vec3(0.6f));

        glm::mat4 modelModel(1.0f); // modelModel 💀
        modelModel = glm::translate(modelModel, glm::vec3(5.0f, 0.0f, 0.0f));
        lightingShader.setMat4("uModel", glm::value_ptr(modelModel));
        model.draw(lightingShader);

        lightCubeShader.use();
        lightCubeShader.setMat4("uProjection", glm::value_ptr(projection));
        lightCubeShader.setMat4("uView", glm::value_ptr(view));

        // // we now draw as many light bulbs as we have point lights.
        // glBindVertexArray(lightCubeVAO);
        // for (unsigned int i = 0; i < 4; i++)
        // {
        //     model = glm::mat4(1.0f);
        //     model = glm::translate(model, pointLightPositions[i]);
        //     model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
        //     lightCubeShader.setMat4("model", glm::value_ptr(model));
        //     glDrawArrays(GL_TRIANGLES, 0, 36);
        // }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(DOWN, deltaTime);
}

// callback for window size change
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.processMouseMovement(xoffset, yoffset);
}
