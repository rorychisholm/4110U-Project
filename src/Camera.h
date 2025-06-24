#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float rotationSpeed;

    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch),
        movementSpeed(5.5f), rotationSpeed(10.5f) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    glm::vec3 getPos() {
        return position;
    }

    void processKeyboard(int key, float deltaTime) {
        float velocity = movementSpeed * deltaTime;
        float rotation = rotationSpeed * deltaTime;

        if (key == GLFW_KEY_W)
            position += front * velocity;  // Move forward
        if (key == GLFW_KEY_S)
            position -= front * velocity;  // Move backward
        if (key == GLFW_KEY_A)
            position -= right * velocity;  // Move left
        if (key == GLFW_KEY_D)
            position += right * velocity;  // Move right

        if (key == GLFW_KEY_UP)
            pitch += rotation;  // Look up
        if (key == GLFW_KEY_DOWN)
            pitch -= rotation;  // Look down
        if (key == GLFW_KEY_LEFT)
            yaw -= rotation;  // Look left
        if (key == GLFW_KEY_RIGHT)
            yaw += rotation;  // Look right

        // Constrain pitch to avoid gimbal lock
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};
