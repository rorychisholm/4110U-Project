#pragma once

#include <iostream>
#include <string>
#include "Object.h"
#include "LandMass.h"
#include <glm/gtc/random.hpp> // For random generation

// Class representing a member (e.g., a bee) in the simulation
class Member : public Object {
private:
    glm::vec3 p; // Current position
    glm::vec3 v; // Current velocity
    glm::vec3 a; // Current acceleration
    glm::vec3 d; // Current direction
    glm::vec3 h; // Hive location - known to bees
    bool returnHome; // Whether the bee should return home
    int pollen; // Amount of pollen collected by the bee

public:
    // Parameterized Constructor
    Member(const std::string& objPath, GLuint shaderProgram, const glm::vec3& color, glm::vec3 position)
        : Object(objPath, shaderProgram, color), p(position), v(1.0, 0.0, 0.0), a(0), d(1), h(position), pollen(0) {
        modelMatrix = glm::translate(glm::mat4(1.0f), p); // Initialize model matrix with position
        returnHome = false; // Default state: not returning home
    }

    // Destructor
    ~Member() {}

    // Update the model matrix for rendering
    void update() {
        modelMatrix = glm::mat4(1.0f); // Reset model matrix

        // Apply translation based on position
        modelMatrix = glm::translate(modelMatrix, p);

        // Apply scaling for visualization
        modelMatrix = glm::scale(modelMatrix, glm::vec3(7.5f));

        // Default "forward" direction of the model
        glm::vec3 defaultDirection = glm::vec3(0.0f, 0.0f, 1.0f);

        // Constrain direction `d` to keep the model upright
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 constrainedD = glm::normalize(glm::vec3(d.x, 0.0f, d.z)); // Project `d` onto the XZ plane

        // Calculate rotation axis between default and constrained directions
        glm::vec3 rotationAxis = glm::cross(defaultDirection, constrainedD);

        // Apply rotation if necessary
        if (glm::length(rotationAxis) > 0.0001f) {
            rotationAxis = glm::normalize(rotationAxis);
            float angle = glm::acos(glm::clamp(glm::dot(defaultDirection, constrainedD), -1.0f, 1.0f));
            modelMatrix = glm::rotate(modelMatrix, angle, rotationAxis);
        }
    }

    // Simulate movement with given forces and constraints
    void move(float deltaTime, std::vector<glm::vec3> ffs, std::vector<glm::vec3> fps, BoundBox bounds, std::vector<std::vector<glm::vec3>> teamPosDir, LandMass land) {
        const float obstacleRadius = 0.5f;      // Avoidance radius for obstacles
        const float avoidanceStrength = 2.5f;  // Strength of obstacle avoidance
        const float maxSpeed = 5.0f;           // Maximum speed
        const float noiseScale = 2.0f;         // Random noise scale
        const float swarmRad = 15.0f;           // Swarm interaction radius
        const float swarmStr = 0.85f;          // Swarm influence strength
        const float objSense = 10.0f;          // Object sensing radius
        const float iterRad = 2.0f;            // Interaction radius
        const float boundRad = 5.0f;           // Boundary radius
        const float contStr = 10.0f;           // Strength of boundary restoring force

        // Calculate team alignment vectors
        std::pair<glm::vec3, glm::vec3> avgPair = teamAvg(teamPosDir, swarmRad);
        glm::vec3 teamAvgPos = glm::length(avgPair.first) > 0.0f ? glm::normalize(avgPair.first) * swarmStr : glm::vec3(0.0f);
        glm::vec3 teamAvgDir = glm::length(avgPair.second) > 0.0f ? glm::normalize(avgPair.second) * swarmStr : glm::vec3(0.0f);

        a = glm::vec3(0.0f); // Reset acceleration

        float r; // Distance to obstacle or target

        if (returnHome) {
            r = glm::length(h - p);
            a += glm::normalize(h - p) * contStr * 0.75f; // Steer toward hive
            if (r < iterRad) {
                returnHome = false;
                color = glm::vec3(1.0f, 0.843f, 0.0f); // Change color to indicate bee is free
                pollen++; // Deposit pollen
            }
        }
        else {
            for (const auto& point : fps) {
                if (!glm::any(glm::isnan(point))) {
                    glm::vec3 offset = point - p;
                    r = glm::length(offset);
                    if (r < objSense && r != 0) {
                        a += glm::normalize(offset) * contStr * 0.75f;
                        if (r < iterRad) {
                            returnHome = true; // Trigger return to hive
                            color = glm::vec3(1.0f, 0.5f, 0.0f); // Change color to indicate return state
                        }
                    }
                }
            }
        }

        // Avoid obstacles
        for (const auto& point : ffs) {
            if (!glm::any(glm::isnan(point))) {
                glm::vec3 offset = p - point;
                r = glm::length(offset);
                if (r < obstacleRadius && r != 0) {
                    a += (offset / (r * r)) * avoidanceStrength;
                }
            }
        }

        // Add boundary repellent forces
        glm::vec3 boundaryForce(0.0f);
        if (p.x < bounds.min.x + boundRad) boundaryForce.x += contStr / (p.x - bounds.min.x);
        if (p.x > bounds.max.x - boundRad) boundaryForce.x -= contStr / (bounds.max.x - p.x);
        if (p.y < bounds.min.y + boundRad) boundaryForce.y += contStr / (p.y - land.getHeight(p.x, p.z));
        if (p.y > bounds.max.y - boundRad) boundaryForce.y -= contStr / (bounds.max.y - p.y);
        if (p.z < bounds.min.z + boundRad) boundaryForce.z += contStr / (p.z - bounds.min.z);
        if (p.z > bounds.max.z - boundRad) boundaryForce.z -= contStr / (bounds.max.z - p.z);

        if (p.x < bounds.min.x || p.x > bounds.max.x
            || p.y > bounds.max.y
            || p.x < bounds.min.z || p.z > bounds.max.z) {
            a += (h - p) * contStr;
        }

        a += boundaryForce;      // Add boundary force

        a += teamAvgPos;         // Add team alignment force

        v += a * deltaTime + teamAvgDir; // Update velocity

        glm::vec3 noise = glm::linearRand(glm::vec3(-1.0f), glm::vec3(1.0f)) * noiseScale;
        v += noise; // Add random noise

        if (glm::length(v) > maxSpeed) {
            v = glm::normalize(v) * maxSpeed; // Limit speed
        }

        p += v * deltaTime; // Update position

        if (p.y < land.getHeight(p.x, p.z)) {
            p -= v * deltaTime * 2.0f; // Reflect off ground
            v.y = -v.y;
        }

        d = glm::normalize(v); // Update direction
    }

    // Render the member
    void display(const glm::mat4 viewMatrix, const glm::mat4 projectionMatrix, const glm::vec3 eyePosition) override {
        Object::display(viewMatrix, projectionMatrix, eyePosition); // Call base class method
    }

    // Get current position
    glm::vec3 getPosition() const {
        return p;
    }

    // Get current direction
    glm::vec3 getDirection() const {
        return d;
    }

    // Set direction with validation
    void setDirection(const glm::vec3& newDirection) {
        if (glm::length(newDirection) > 0.0001f) {
            d = glm::normalize(newDirection);
        }
        else {
            std::cerr << "Error: Attempted to set a zero-length direction vector." << std::endl;
        }
    }

    // Retrieve and reset pollen count
    int getPollen() {
        int pln = pollen;
        pollen = 0;
        return pln;
    }

private:
    // Calculate team average position and direction within a radius
    std::pair<glm::vec3, glm::vec3> teamAvg(std::vector<std::vector<glm::vec3>> posDir, float r) {
        glm::vec3 sumPos(0.0f);
        glm::vec3 sumDir(0.0f);
        float size = 0;
        for (size_t i = 0; i < posDir[0].size(); ++i) {
            if (glm::length(posDir[0][i] - p) < r) {
                sumPos += posDir[0][i];
                sumDir += posDir[1][i];
                size++;
            }
        }
        return std::make_pair(sumPos / size, sumDir / size); // Return average
    }
};
