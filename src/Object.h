#pragma once
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
#include "tiny_obj_loader.h"
#include <vector>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

struct BoundBox {
    glm::vec3 min; // Minimum corner (e.g., bottom-left-back)
    glm::vec3 max; // Maximum corner (e.g., top-right-front)
};

// Class for managing a single renderable object
class Object {
public:
    GLuint VAO;                // Vertex Array Object
    GLuint VBO;                // Vertex Buffer Object
    GLuint EBO;                // Element Buffer Object
    GLuint program;            // Shader program
    GLfloat* vertices;
    GLfloat* normals;
    GLuint* indices;
    int triangleCount;         // Number of triangles
    glm::mat4 modelMatrix;     // Model transformation matrix
    glm::vec3 color;           // Object color


    // Constructor
    Object(const std::string& objPath, GLuint shaderProgram, const glm::vec3& color)
        : VAO(0), VBO(0), EBO(0), program(shaderProgram), triangleCount(0), modelMatrix(1.0f), color(color) {
        init(objPath);
    }

    // Destructor to clean up resources
    ~Object() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
        delete[] vertices;
        delete[] normals;
        delete[] indices;
    }

    // Load OBJ file and set up buffers
    void init(const std::string& objPath) {
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        int nv;
        int nn;
        int ni;
        int i;
        int fid;
        int result;
        struct _stat buf;

        // Generate and bind VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        
        std::string binFPath ="src/" + objPath + ".bin";
        std::string objFPath ="src/" + objPath + ".obj";

        result = _stat(binFPath.c_str(), &buf);

        if (result == 0) {
            // Binary file exists, load it
            fid = _open(binFPath.c_str(), _O_RDONLY | _O_BINARY);
            if (fid == -1) {
                std::cerr << "Failed to open binary file: " << binFPath << std::endl;
                return;
            }

            // Read data from the binary file

            result = _read(fid, &nv, (sizeof vertices));
            result = _read(fid, &nn, (sizeof normals));
            result = _read(fid, &ni, (sizeof indices));
            triangleCount = ni / 3;

            vertices = new GLfloat[nv];
            result = _read(fid, vertices, nv * (sizeof GLfloat));
            normals = new GLfloat[nn];
            result = _read(fid, normals, nn * (sizeof GLfloat));
            indices = new GLuint[ni];
            result = _read(fid, indices, ni * (sizeof GLuint));
            _close(fid);

        }
        else {
            // Load from OBJ file
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;

            std::string err = tinyobj::LoadObj(shapes, materials, objFPath.c_str(), 0);

            if (!err.empty()) {
                std::cerr << err << std::endl;
                return;
            }
            
            /*  Retrieve the vertex coordinate data */

            nv = (int)shapes[0].mesh.positions.size();
            vertices = new GLfloat[nv];
            for (i = 0; i < nv; i++) {
                vertices[i] = shapes[0].mesh.positions[i];
            }

            /*  Retrieve the vertex normals */

            nn = (int)shapes[0].mesh.normals.size();
            normals = new GLfloat[nn];
            for (i = 0; i < nn; i++) {
                normals[i] = shapes[0].mesh.normals[i];
            }

            /*  Retrieve the triangle indices */

            ni = (int)shapes[0].mesh.indices.size();
            triangleCount = ni / 3;
            indices = new GLuint[ni];
            for (i = 0; i < ni; i++) {
                indices[i] = shapes[0].mesh.indices[i];
            }

            // save the binary version of the model

            fid = _open(binFPath.c_str(), _O_WRONLY | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE);

            result = _write(fid, &nv, (sizeof vertices));
            result = _write(fid, &nn, (sizeof normals));
            result = _write(fid, &ni, (sizeof indices));

            result = _write(fid, vertices, nv * (sizeof GLfloat));
            result = _write(fid, normals, nn * (sizeof GLfloat));
            result = _write(fid, indices, ni * (sizeof GLuint));

            _close(fid);
        }

        // Upload data to GPU
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (nv + nn) * sizeof(GLfloat), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, nv * sizeof(GLfloat), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, nv * sizeof(GLfloat), nn * sizeof(GLfloat), normals);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni * sizeof(GLuint), indices, GL_STATIC_DRAW);

        // Link vertex attributes
        GLint vPosition = glGetAttribLocation(program, "vPosition");
        glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(vPosition);

        GLint vNormal = glGetAttribLocation(program, "vNormal");
        glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void*>(nv * sizeof(GLfloat)));
        glEnableVertexAttribArray(vNormal);

        // Unbind VAO
        glBindVertexArray(0);

    }


    // Render the object
    virtual void display(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const glm::vec3 eyePosition) {
        glUseProgram(program);

        // Set shader uniforms
        GLint modelLoc = glGetUniformLocation(program, "modelView");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix * modelMatrix));

        GLint projLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        GLint eyeLoc = glGetUniformLocation(program, "eye");
        glUniform3fv(eyeLoc, 1, glm::value_ptr(eyePosition));

        GLint colorLoc = glGetUniformLocation(program, "objectColor");
        glUniform3fv(colorLoc, 1, glm::value_ptr(color)); // Pass the color to the shader

        // Draw the object
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, triangleCount * 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    std::vector<glm::vec3> genFFs(float minHeight = 1.0f) {
        std::vector<glm::vec3> ffs;

        // Loop through all vertices
        for (int i = 0; i < triangleCount * 3; ++i) {
            // Fetch the vertex index from the indices array
            int vertexIndex = indices[i]; // Use indices to reference the correct vertex
            int baseIndex = vertexIndex * 3; // Each vertex has 3 components (x, y, z)

            // Get the vertex coordinates
            float x = vertices[baseIndex + 0];
            float y = vertices[baseIndex + 1];
            float z = vertices[baseIndex + 2];

            auto point = roundTo3(glm::vec3(x, y, z));

            // Only consider vertices above the minimum height
            if (y > minHeight && std::find(ffs.begin(), ffs.end(), point) == ffs.end()) {
                ffs.emplace_back(point); // Add obstacle point
            }
        }
        return ffs;
    }

    BoundBox getBounds(float minHeight = 0.0f){
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        BoundBox boundary;

        // Loop through all vertices
        for (int i = 0; i < triangleCount * 3; ++i) {
            // Fetch the vertex index from the indices array
            int vertexIndex = indices[i]; // Use indices to reference the correct vertex
            int baseIndex = vertexIndex * 3; // Each vertex has 3 components (x, y, z)

            // Get the vertex coordinates
            float x = vertices[baseIndex + 0];
            float y = vertices[baseIndex + 1];
            float z = vertices[baseIndex + 2];

            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (z < minZ) minZ = z;
            if (z > maxZ) maxZ = z;
        }
        boundary.min = glm::vec3(minX, minHeight, minZ);
        boundary.max = glm::vec3(maxX, (maxX + maxZ) / 4, maxZ);
        return boundary;
    }

    void printVec3(const glm::vec3& vec, const std::string& label = "Vector") {
        std::cout << label << ": ("
            << vec.x << ", "
            << vec.y << ", "
            << vec.z << ")" << std::endl;
    }

    glm::vec3 roundTo3(const glm::vec3& point) {
        return glm::vec3(
            std::round(point.x * 1000.0f) / 1000.0f,
            std::round(point.y * 1000.0f) / 1000.0f,
            std::round(point.z * 1000.0f) / 1000.0f
        );
    }

};
