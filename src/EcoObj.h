#pragma once

#include <iostream>
#include <string>
#include "Object.h"
#include <glm/gtc/random.hpp> // For random generation

class EcoObj : public Object {
private:
    glm::vec3 p; // Current position
    glm::vec3 d; // Current direction

public:
    // Parameterized Constructor
    EcoObj(const std::string& objPath, GLuint shaderProgram, const glm::vec3& color, glm::vec3 position, glm::vec3 dir)
        : Object(objPath, shaderProgram, color), p(position){
        modelMatrix = glm::translate(glm::mat4(1.0f), p);
        setDirection(dir);
    }

    // Destructor
    ~EcoObj() {}

    void display(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const glm::vec3 eyePosition) override {
        // Call base class display method with updated model matrix
        Object::display(viewMatrix, projectionMatrix, eyePosition);
    }

    // Getter function for the object's position
    glm::vec3 getPosition() const {
        return p;
    }

    // Getter function for the object's direction
    glm::vec3 getDirection() const {
        return d;
    }

    // Getter function for the object's direction
    void setDirection(glm::vec3 d) {
        if (glm::length(d) > 0.0001f) { // Avoid setting zero-length direction
            this->d = glm::normalize(d);     // Normalize the direction

            modelMatrix = glm::mat4(1.0f);

            // Apply translation to position
            modelMatrix = glm::translate(modelMatrix, p);

            // Default model "forward" direction (e.g., along the -Z axis)
            glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, 1.0f);

            // Ensure direction `d` is normalized
            glm::vec3 normalizedD = glm::normalize(d);

            // Calculate the rotation axis
            glm::vec3 rotationAxis = glm::cross(defaultDirection, normalizedD);

            // Handle case where vectors are parallel
            if (glm::length(rotationAxis) > 0.0001f) {
                rotationAxis = glm::normalize(rotationAxis);

                // Calculate the angle between the directions using the dot product
                float angle = glm::acos(glm::clamp(glm::dot(defaultDirection, normalizedD), -1.0f, 1.0f)); // Clamp to avoid NaN

                // Apply the rotation
                modelMatrix = glm::rotate(modelMatrix, angle, rotationAxis);
            }
        }
        else {
            std::cerr << "Error: Attempted to set a zero-length direction vector." << std::endl;
        }
    }

    void setPosition(glm::vec3 p) {
        p = p; // Set the position
    }

    void setSize(float scale) {
        // Apply scaling
        setDirection(d);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        
    }

    void turnUpsideDown() {
        // Default direction for the model (e.g., facing forward along -Z)
        glm::vec3 defaultDirection = glm::normalize(d); // Current direction
        glm::vec3 upsideDownDirection = -defaultDirection; // Invert the direction

        // Calculate the rotation axis
        glm::vec3 rotationAxis = glm::cross(defaultDirection, upsideDownDirection);

        // Handle the edge case where the vectors are parallel or anti-parallel
        if (glm::length(rotationAxis) < 0.0001f) {
            // Use a fallback axis if the vectors are parallel (e.g., rotate around X)
            rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        }
        else {
            rotationAxis = glm::normalize(rotationAxis);
        }

        // 180 degrees (π radians) rotation
        float angle = glm::radians(180.0f);

        // Apply the rotation to the model matrix
        modelMatrix = glm::rotate(modelMatrix, angle, rotationAxis);
    }
};