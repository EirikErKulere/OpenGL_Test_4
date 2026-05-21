#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "loadTexture.h"

// #define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);

unsigned int screenWidth = 1300;
unsigned int screenHeight = 850;

int fbWidth, fbHeight;
unsigned int textureColorBuffer;

Camera camera;
float lastX = screenWidth  / 2.0f;
float lastY = screenHeight / 2.0f;
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

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL Test 4", NULL, NULL);
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

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // std::cout << "fbWidth: " << fbWidth << std::endl;
    // std::cout << "fbHeight: " << fbHeight << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // SHADERS
    Shader lightingShader("../resources/shaders/block.vert", "../resources/shaders/block.frag");
    Shader lightCubeShader("../resources/shaders/light_cube.vert", "../resources/shaders/light_cube.frag");
    Shader screenShader("../resources/shaders/screen.vert", "../resources/shaders/screen.frag");
    Shader skyboxShader("../resources/shaders/skybox.vert", "../resources/shaders/skybox.frag");
    // Shader mirrorShader("../resources/shaders/block.vert", "../resources/shaders/mirror.frag");

    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float screenVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int screenVAO, screenVBO;
    glGenVertexArrays(1, &screenVAO);
    glGenBuffers(1, &screenVBO);
    glBindVertexArray(screenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float vertices[] = {
         // positions         // normals           // texture coordinates
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

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
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
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
    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // glm::vec3 pointLightPositions[] = {
    //     glm::vec3( 0.7f,  0.2f,  2.0f),
    //     glm::vec3( 2.3f, -3.3f, -4.0f),
    //     glm::vec3(-4.0f,  2.0f, -12.0f),
    //     glm::vec3( 0.0f,  0.0f, -3.0f)
    // };
    glm::vec3 grassPositions[] = {
        glm::vec3(-1.5f, -4.0f, -0.48f),
        glm::vec3( 1.5f, -4.0f,  0.51f),
        glm::vec3( 0.0f, -4.0f,  0.7f),
        glm::vec3(-0.3f, -4.0f, -2.3f),
        glm::vec3( 0.5f, -4.0f, -0.6f)
    };

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

    // SKYBOX
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // GROUND MESH
    std::vector<Vertex> groundVertices;
    //                        position                            normal                       texture coordinates
    groundVertices.push_back({glm::vec3(-100.0f, -4.0f, -100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(  0.0f,   0.0f)});
    groundVertices.push_back({glm::vec3( 100.0f, -4.0f, -100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(100.0f,   0.0f)});
    groundVertices.push_back({glm::vec3( 100.0f, -4.0f,  100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(100.0f, 100.0f)});
    groundVertices.push_back({glm::vec3(-100.0f, -4.0f,  100.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(  0.0f, 100.0f)});
    std::vector<unsigned int> groundIndices{0, 2, 1, 0, 3, 2};
    std::vector<Texture> groundTextures{{
        loadTexture(std::string("../resources/textures/grid.png").c_str(), []() {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }),
        "texture_diffuse"
    }};
    glm::vec4 groundColor(0.0f);
    Mesh ground(groundVertices, groundIndices, groundTextures, groundColor);

    // GRASS MESH
    std::vector<Vertex> grassVertices;
    grassVertices.push_back({glm::vec3(-0.5f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)});
    grassVertices.push_back({glm::vec3( 0.5f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)});
    grassVertices.push_back({glm::vec3( 0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)});
    grassVertices.push_back({glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)});

    // grassVertices.push_back({glm::vec3( 0.5f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)});
    // grassVertices.push_back({glm::vec3(-0.5f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)});
    // grassVertices.push_back({glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)});
    // grassVertices.push_back({glm::vec3( 0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)});
    // std::vector<unsigned int> grassIndices{0, 2, 1, 0, 3, 2, 4, 6, 5, 4, 7, 6};
    std::vector<unsigned int> grassIndices{0, 2, 1, 0, 3, 2};
    std::vector<Texture> grassTextures{{
        loadTexture(std::string("../resources/textures/blending_transparent_window.png").c_str(), []() {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }),
        "texture_diffuse"
    }};
    glm::vec4 grassColor(0.0f);
    Mesh grass(grassVertices, grassIndices, grassTextures, grassColor);

    // OSKAR DONUT
    Model model(std::string("../resources/3d-models/donut_oskar/donut_oskar.obj").c_str());

    unsigned int diffuseMap = loadTexture(std::string("../resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(std::string("../resources/textures/container2_specular.png").c_str());

    // std::vector<std::string> faces {
    //     "../resources/textures/skybox/right.jpg",
    //     "../resources/textures/skybox/left.jpg",
    //     "../resources/textures/skybox/top.jpg",
    //     "../resources/textures/skybox/bottom.jpg",
    //     "../resources/textures/skybox/front.jpg",
    //     "../resources/textures/skybox/back.jpg",
    // };

    std::vector<std::string> faces {
        "../resources/textures/clouds1/clouds1_east.bmp",
        "../resources/textures/clouds1/clouds1_west.bmp",
        "../resources/textures/clouds1/clouds1_up.bmp",
        "../resources/textures/clouds1/clouds1_down.bmp",
        "../resources/textures/clouds1/clouds1_north.bmp",
        "../resources/textures/clouds1/clouds1_south.bmp",
    };

    unsigned int skyboxTexture = loadCubemap(faces);

    skyboxShader.use();
    skyboxShader.setInt("uCubeMap", 0);

    lightingShader.use();
    // lightingShader.setInt("uMaterial.diffuse", 0);
    // lightingShader.setInt("uMaterial.specular", 1);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        // glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)fbWidth / (float)fbHeight, 0.1f, 500.0f);
        glm::mat4 view = camera.getViewMatrix();

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

        // GROUND
        lightingShader.setVec3("uSunlight.ambient", glm::vec3(0.3f));
        lightingShader.setVec3("uSunlight.diffuse", glm::vec3(1.0f));
        lightingShader.setVec3("uSunlight.specular", glm::vec3(0.4f));
        glm::mat4 groundModel(1.0f);
        lightingShader.setMat4("uModel", glm::value_ptr(groundModel));
        ground.draw(lightingShader);
        lightingShader.setVec3("uSunlight.ambient", glm::vec3(0.5f));
        lightingShader.setVec3("uSunlight.diffuse", glm::vec3(1.4f));
        lightingShader.setVec3("uSunlight.specular", glm::vec3(0.6f));

        // OSKAR DONUT
        glDisable(GL_CULL_FACE);
        glm::mat4 modelModel(1.0f); // modelModel 💀
        modelModel = glm::translate(modelModel, glm::vec3(5.0f, 0.0f, 0.0f));
        lightingShader.setMat4("uModel", glm::value_ptr(modelModel));
        model.draw(lightingShader);
        glEnable(GL_CULL_FACE);

        // lightCubeShader.use();
        // lightCubeShader.setMat4("uProjection", glm::value_ptr(projection));
        // lightCubeShader.setMat4("uView", glm::value_ptr(view));

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

        // SKYBOX
        skyboxShader.use();
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glDisable(GL_CULL_FACE);
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.getViewMatrix()));
        skyboxShader.setMat4("uView", glm::value_ptr(skyboxView));
        skyboxShader.setMat4("uProjection", glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        // GRASS
        lightingShader.use();
        glDisable(GL_CULL_FACE);
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < (sizeof(grassPositions)/sizeof(grassPositions[0])); i++) {
            float distance = glm::length(camera.position - grassPositions[i]);
            sorted[distance] = grassPositions[i];
        }
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            glm::mat4 grassModel(1.0f);
            grassModel = glm::translate(grassModel, it->second);
            lightingShader.setMat4("uModel", glm::value_ptr(grassModel));
            grass.draw(lightingShader);
        }
        glEnable(GL_CULL_FACE);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        screenShader.use();
        screenShader.setInt("uTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &screenVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &screenVBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &skyboxTexture);
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
    screenWidth = width;
    screenHeight = height;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, width, height);
    // glViewport(0, 0, fbWidth, fbHeight);

    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);
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

unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
