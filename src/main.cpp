/************************************************
 *
 *       CSCI 4110 Project - Bee Simulation
 *       William Rory Chisholm - 100560820
 *
 ************************************************/

 // Standard library and OpenGL includes
#include <Windows.h>
#include <gl/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include <stdio.h>
#include "tiny_obj_loader.h"
#include <iostream>
#include "Camera.h"
#include "Object.h"
#include "Member.h"
#include "LandMass.h"
#include "EcoObj.h"
#include <vector>
#include <chrono>
#include <thread>

// Global variables for shader program, object data, and other parameters
GLuint program;      // Shader program ID
GLuint objVAO;       // Vertex Array Object for the object
int striangles;      // Number of triangles
GLuint sibuffer;     // Buffer for indices
int window;          // Window identifier
Camera camera(glm::vec3(0.0f, 25.0f, 100.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -10.0f); // Camera object

char* vertexName;    // Vertex shader name
char* fragmentName;  // Fragment shader name

// Variables for spherical coordinates
double theta, phi;
double r;

// Update delta for animation
double delta = 0.1;

// Camera position
float cx, cy, cz;

// Matrices for view and projection
glm::mat4 view;       // View matrix
glm::mat4 projection; // Projection matrix

// Function to handle window resizing
void framebufferSizeCallback(GLFWwindow* window, int w, int h) {
    if (h == 0) h = 1; // Prevent division by zero if height is zero

    float ratio = 1.0f * w / h; // Aspect ratio

    glfwMakeContextCurrent(window);

    glViewport(0, 0, w, h); // Set the viewport dimensions

    // Create a perspective projection matrix
    projection = glm::perspective(0.7f, ratio, 1.0f, 800.0f);
}

// Function to handle keyboard input
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE); // Close window on Escape key press

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        camera.processKeyboard(key, 0.1); // Process camera movement
    }
}

// Callback for GLFW errors
void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

// Function to extract positions and directions from a team of objects
std::vector<std::vector<glm::vec3>> getTeamLocDir(std::vector<std::shared_ptr<Member>> team) {
    std::vector<glm::vec3> teamLoc;
    std::vector<glm::vec3> teamDir;
    std::vector<std::vector<glm::vec3>> teamLocDir;

    for (const auto& mem : team) {
        teamLoc.emplace_back(mem->getPosition());   // Get positions
        teamDir.emplace_back(mem->getDirection()); // Get directions
    }
    teamLocDir.emplace_back(teamLoc);
    teamLocDir.emplace_back(teamDir);
    return teamLocDir;
}

int main(int argc, char** argv) {
    auto timer = std::chrono::steady_clock::now(); // Timer for tracking execution
    std::srand(static_cast<unsigned int>(std::time(0)));
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback); // Set error callback

    if (!glfwInit()) { // Initialize GLFW
        fprintf(stderr, "can't initialize GLFW\n");
    }

    // Create a GLFW window
    window = glfwCreateWindow(512, 512, "Adv Graphics Project - Bee Sim", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Set callbacks for window resizing and key input
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    GLenum error = glewInit(); // Initialize GLEW
    if (error != GLEW_OK) {
        printf("Error starting GLEW: %s\n", glewGetErrorString(error));
        exit(0);
    }

    // Load shaders and create shader program
    GLuint vs = buildShader(GL_VERTEX_SHADER, "src/lab1c.vs");
    GLuint fs = buildShader(GL_FRAGMENT_SHADER, "src/lab1c.fs");
    GLuint shaderProgram = buildProgram(vs, fs, 0);

    // Initialize variables
    int n = 8; // Grid size
    int gridSize = pow(n, 2) + 1;
    float halfGrid = gridSize / 2;
    glm::vec3 hiveLoc = glm::vec3(0.0f);

    // Initialize landmass and other objects
    LandMass land(n, shaderProgram);

    BoundBox bounds = land.getBounds(); // Get land boundaries
    hiveLoc.y = land.getHeight(hiveLoc.x, hiveLoc.z) + 2;
    EcoObj hive("hive", shaderProgram, glm::vec3(1.0f, 0.627f, 0.196f), hiveLoc, glm::vec3(0, 0, 1));

    std::vector<std::shared_ptr<Member>> swarm; // Vector for swarm (bees)
    std::vector<std::shared_ptr<EcoObj>> flowers; // Vector for flowers

    // Create initial swarm of bees
    for (int i = 0; i < 25; ++i) {
        swarm.emplace_back(std::make_shared<Member>("bee", shaderProgram, glm::vec3(1.0f, 0.843f, 0.0f), hiveLoc));
    }

    std::vector<glm::vec3> flowerPts; // Vector for flower points

    // Generate initial flowers
    for (int i = 0; i < 50; ++i) {
        glm::vec3 fp;
        do{
            fp = glm::linearRand(bounds.min + glm::vec3(1.0f), bounds.max - glm::vec3(1.0f));
        } while (glm::length(fp) < 10.0f); // Re-randomize if within 10 units of the origin
        fp.y = land.getHeight(fp.x, fp.z) + 0.5f;
        flowerPts.push_back(fp);
        flowers.emplace_back(std::make_shared<EcoObj>("flower_platform", shaderProgram, glm::vec3(1.0f, 0.4118f, 0.7059f), fp, glm::vec3(0, 0, 1)));
    }

    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glClearColor(0.529f, 0.808f, 0.922f, 1.0); // Set clear color
    glViewport(0, 0, 512, 512); // Set viewport dimensions

    projection = glm::perspective(0.7f, 1.0f, 1.0f, 800.0f); // Set initial projection matrix

    glfwSwapInterval(3); // Limit frame rate

    auto beeTimer = std::chrono::steady_clock::now(); // Timer for bee updates
    auto flowerTimer = std::chrono::steady_clock::now(); // Timer for flower updates

    int plnCount = 0; // Pollen count

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear buffers

        view = camera.getViewMatrix(); // Update view matrix
        projection = glm::perspective(0.7f, 1.0f, 1.0f, 800.0f); // Update projection matrix

        auto currentTime = std::chrono::steady_clock::now(); // Current time

        // Spawn new bee periodically
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - beeTimer).count() >= 60 / std::sqrt(plnCount + 1) && swarm.size() < 200) {
            swarm.emplace_back(std::make_shared<Member>("bee", shaderProgram, glm::vec3(1.0f, 0.843f, 0.0f), hiveLoc));
            beeTimer = currentTime;
            if (swarm.size() == 200) {
                std::cout << "A New Bee was Born!! Max Beez!!!" << std::endl;
                std::cout << std::chrono::duration_cast<std::chrono::seconds>(currentTime - timer).count() << std::endl;
            }
            else {
                std::cout << "A New Bee was Born!!" << std::endl;
            }
        }

        // Manage flower lifecycle
        if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - flowerTimer).count() >= 10) {
            if (!flowerPts.empty()) {
                flowerPts.erase(flowerPts.begin()); // Remove the first flower
                flowers.erase(flowers.begin());
            }
            glm::vec3 newFlower;
            do {
               newFlower = glm::linearRand(bounds.min + glm::vec3(1.0f), bounds.max - glm::vec3(1.0f));
            } while (glm::length(newFlower) < 10.0f); // Re-randomize if within 10 units of the origin
            newFlower.y = land.getHeight(newFlower.x, newFlower.z) + 0.5f;
            flowerPts.push_back(newFlower); // Add new flower point

            flowers.emplace_back(std::make_shared<EcoObj>("flower_platform", shaderProgram, glm::vec3(1.0f, 0.4118f, 0.7059f), newFlower, glm::vec3(0, 0, 1)));
            flowerTimer = currentTime;
            std::cout << "As one blossom withers, another blooms." << std::endl;
        }

        // Update and render swarm
        std::vector<std::vector<glm::vec3>> swarmLocDir = getTeamLocDir(swarm);
        std::vector<glm::vec3> tempFF = swarmLocDir[0];

        for (const auto& bee : swarm) {
            bee->move(delta, tempFF, flowerPts, bounds, swarmLocDir, land); // Move bee
            bee->update(); // Update bee state
            bee->display(view, projection, camera.getPos()); // Render bee
            if (plnCount < 500) {
                plnCount += bee->getPollen();
                if (plnCount == 500) {
                    std::cout << "Max Hive!!!" << std::endl;
                    std::cout << std::chrono::duration_cast<std::chrono::seconds>(currentTime - timer).count() << std::endl;
                }
            }
        }

        // Update and render flowers
        for (const auto& flower : flowers) {
            flower->setSize(0.0035); // Set flower size
            flower->display(view, projection, camera.getPos()); // Render flower
        }

        // Render hive and land
        hive.setSize(2.5 + (plnCount * 0.01)); // Update hive size
        hive.turnUpsideDown(); // Animate hive
        hive.display(view, projection, camera.getPos()); // Render hive
        land.display(view, projection, camera.getPos()); // Render land

        glfwSwapBuffers(window); // Swap front and back buffers
        glfwPollEvents(); // Poll for and process events
    }

    glfwTerminate(); // Clean up and terminate GLFW
    return 0;
}