#pragma once

#include "Object.h"
#include <ctime>
#include <random>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class LandMass {
private:
    int width;               // Width of the grid
    int height;              // Height of the grid
    int gridSize;            // Size of the grid (number of points per side)
    float displace;          // Displacement scale for terrain generation
    int triangleCount;       // Number of triangles in the grid

    GLuint program;          // Shader program used for rendering
    GLuint planeVAO;         // Vertex Array Object for the plane
    GLuint planeBuffer;      // Buffer for plane indices

    glm::mat4 modelMatrix;   // Model transformation matrix

    std::vector<std::vector<float>> heightGrid; // 2D grid of height values
    std::mt19937 generator;                     // Random number generator
    std::uniform_real_distribution<float> floatDistribution; // Distribution for random numbers

    // Helper function to scale a value from one range to another
    float scaleValue(float value, float originalMin, float originalMax, float targetMin, float targetMax) {
        return targetMin + ((value - originalMin) / (originalMax - originalMin)) * (targetMax - targetMin);
    }

    // Function to generate terrain using the diamond-square algorithm
    std::vector<std::vector<float>> fractleGen(int size) {
        // Initialize a 2D grid of heights with zero
        std::vector<std::vector<float>> heightGrid(size, std::vector<float>(size, 0.0f));
        float scale = displace; // Initial displacement scale
        int step = size / 2;    // Initial step size

        std::mt19937 generator(static_cast<unsigned int>(std::time(0))); // Seed random generator
        std::uniform_real_distribution<float> floatDistribution(-scale, scale); // Random value distribution

        // Try loading a height map from a file
        std::ifstream inputFile("src/Height_Map.txt");
        std::string line;

        if (inputFile.is_open()) {
            std::cout << "File found, Loading..." << std::endl;
            std::vector<std::vector<float>> map;
            while (getline(inputFile, line)) {
                std::istringstream iss(line);
                std::vector<float> row;
                int value;
                while (iss >> value) {
                    float scaledValue = scaleValue(value, 1, 5, 0, scale);
                    row.push_back(scaledValue);
                }
                map.push_back(row);
            }

            // Scale and populate the grid with values from the map
            for (int i = 0; i < map.size(); ++i) {
                for (int j = 0; j < map[i].size(); ++j) {
                    heightGrid[i * 3][j * 3] = map[i][j];
                }
            }

            inputFile.close();
        }
        else {
            std::cout << "File not found, Randomizing..." << std::endl;

            // Initialize corners with random values
            heightGrid[0][0] = floatDistribution(generator);
            heightGrid[0][size - 1] = floatDistribution(generator);
            heightGrid[size - 1][0] = floatDistribution(generator);
            heightGrid[size - 1][size - 1] = floatDistribution(generator);
        }

        // Perform diamond-square steps to fill in the grid
        while (step > 1) {
            int half = step / 2;

            // Diamond step
            for (int x = 0; x < size - 1; x += step) {
                for (int y = 0; y < size - 1; y += step) {
                    if (heightGrid[x + half][y + half] == 0.0) {
                        float diamondAverage = (heightGrid[x][y] +
                            heightGrid[x + step][y] +
                            heightGrid[x][y + step] +
                            heightGrid[x + step][y + step]) / 4;
                        heightGrid[x + half][y + half] = diamondAverage + floatDistribution(generator);
                    }
                }
            }

            // Square step
            for (int x = 0; x < size; x += half) {
                for (int y = (x + half) % step; y < size; y += step) {
                    if (heightGrid[x][y] == 0.0) {
                        float squareAverage = 0.0f;
                        int count = 0;

                        if (x - half >= 0) {
                            squareAverage += heightGrid[x - half][y];
                            count++;
                        }
                        if (x + half < size) {
                            squareAverage += heightGrid[x + half][y];
                            count++;
                        }
                        if (y - half >= 0) {
                            squareAverage += heightGrid[x][y - half];
                            count++;
                        }
                        if (y + half < size) {
                            squareAverage += heightGrid[x][y + half];
                            count++;
                        }

                        squareAverage /= count;
                        heightGrid[x][y] = squareAverage + floatDistribution(generator);
                    }
                }
            }

            step /= 2;      // Reduce step size
            scale /= 2;     // Reduce displacement scale
            floatDistribution = std::uniform_real_distribution<float>(0.0f, scale); // Update random range
        }
        return heightGrid;
    }

public:
    // Constructor for LandMass class
    LandMass(int n, GLuint shaderProgram)
        : program(shaderProgram),
        gridSize(pow(2, n) + 1),
        displace(pow(1.25, n)),
        modelMatrix(1.0f),
        generator(static_cast<unsigned int>(std::time(0))),
        floatDistribution(-displace, displace) {

        width = 512;  // Set width of the viewport
        height = 512; // Set height of the viewport

        GLuint vbuffer; // Vertex buffer object
        GLint vPosition; // Position attribute location
        GLint vNormal;   // Normal attribute location

        int vs; // Vertex shader
        int fs; // Fragment shader

        // Initialize vertex and normal data
        std::vector<GLfloat> vertices(gridSize * gridSize * 4);
        std::vector<GLfloat> normals(gridSize * gridSize * 3);
        std::vector<GLuint> indices((gridSize - 1) * (gridSize - 1) * 6);
        triangleCount = (gridSize - 1) * (gridSize - 1) * 6; // Calculate total triangles

        vs = buildShader(GL_VERTEX_SHADER, "src/assign1.vs"); // Compile vertex shader
        fs = buildShader(GL_FRAGMENT_SHADER, "src/assign1.fs"); // Compile fragment shader
        program = buildProgram(vs, fs, 0); // Link shader program

        heightGrid = fractleGen(gridSize); // Generate height grid

        // Populate vertex and normal data
        for (int i = 0; i < gridSize; i++) {
            for (int j = 0; j < gridSize; j++) {
                int index = (i * gridSize + j) * 4;
                int nIndex = (i * gridSize + j) * 3;

                // Center the grid by shifting coordinates
                vertices[index] = i - gridSize / 2.0f; // x-coordinate
                vertices[index + 1] = (i == 0 || i == gridSize - 1 || j == 0 || j == gridSize - 1) ? 0 : heightGrid[i][j]; // y-coordinate (height)
                vertices[index + 2] = j - gridSize / 2.0f; // z-coordinate
                vertices[index + 3] = 1.0f;

                normals[nIndex] = 0.0f;       // x-component of normal
                normals[nIndex + 1] = 1.0f;   // y-component of normal
                normals[nIndex + 2] = 0.0f;   // z-component of normal
            }
        }

        // Populate indices for triangles
        int idx = 0;
        for (int i = 0; i < gridSize - 1; i++) {
            for (int j = 0; j < gridSize - 1; j++) {
                GLuint topLeft = i * gridSize + j;
                GLuint topRight = topLeft + 1;
                GLuint bottomLeft = topLeft + gridSize;
                GLuint bottomRight = bottomLeft + 1;

                indices[idx++] = topLeft;
                indices[idx++] = bottomLeft;
                indices[idx++] = bottomRight;

                indices[idx++] = topLeft;
                indices[idx++] = bottomRight;
                indices[idx++] = topRight;
            }
        }

        // Create and bind VAO and VBO for the plane
        glGenVertexArrays(1, &planeVAO);
        glBindVertexArray(planeVAO);

        glGenBuffers(1, &vbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat) + normals.size() * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(GLfloat), vertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), normals.size() * sizeof(GLfloat), normals.data());

        glGenBuffers(1, &planeBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        // Bind vertex attributes
        glUseProgram(program);

        vPosition = glGetAttribLocation(program, "vPosition");
        glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(vPosition);

        vNormal = glGetAttribLocation(program, "vNormal");
        glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertices.size() * sizeof(GLfloat)));
        glEnableVertexAttribArray(vNormal);
    }

    // Display function to render the LandMass
    void display(const glm::mat4 viewMat, const glm::mat4 projectionMat, const glm::vec3 eyePosition) {
        glUseProgram(program);

        GLint modelLoc = glGetUniformLocation(program, "modelView");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(viewMat * modelMatrix));

        GLint projLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMat));

        GLint eyeLoc = glGetUniformLocation(program, "eye");
        glUniform3fv(eyeLoc, 1, glm::value_ptr(eyePosition));

        GLint displaceLoc = glGetUniformLocation(program, "displace");
        glUniform1f(displaceLoc, displace);

        glViewport(0, 0, width, height);

        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeBuffer);
        glDrawElements(GL_TRIANGLES, triangleCount, GL_UNSIGNED_INT, NULL);
    }

    // Getter for grid size
    int getGridSize() const {
        return gridSize;
    }

    // Calculate bounding box for the LandMass
    BoundBox getBounds(float minHeight = 0.0f) {
        float halfGridSize = gridSize / 2.0f; // Half of the grid size

        // Define boundaries
        float minX = -halfGridSize;
        float maxX = halfGridSize;
        float minZ = -halfGridSize;
        float maxZ = halfGridSize;

        BoundBox boundary;
        boundary.min = glm::vec3(minX, minHeight, minZ);
        boundary.max = glm::vec3(maxX, displace * displace, maxZ);

        return boundary;
    }

    // Get height at a specific (x, z) location
    float getHeight(float x, float z) {
        float halfGridSize = gridSize / 2.0f; // Center the grid

        // Convert world coordinates to grid indices
        int gridX = static_cast<int>(x + halfGridSize);
        int gridZ = static_cast<int>(z + halfGridSize);

        // Ensure coordinates are within bounds
        if (gridX < 0 || gridX >= gridSize || gridZ < 0 || gridZ >= gridSize) {
            return 0.0f; // Return 0.0 for out-of-bounds queries
        }

        // Return height from the grid
        return heightGrid[gridX][gridZ];
    }
};
