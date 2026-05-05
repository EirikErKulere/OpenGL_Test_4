#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <utility>

enum movementDirection {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    glm::vec3 position = glm::vec3(2.0f, 3.0f, 2.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front;
    glm::vec3 frontHorizontal;
    glm::vec3 up;
    glm::vec3 right;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 4.0f;
    float sensitivity = 0.1f;

    Camera() {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    // if front x, y or z is 0, we are cooked. might fix later.
    bool isLookingAt(const glm::vec3 &boxMin, const glm::vec3 &boxMax, float &dist) {
        // P(t) = origin + dir * t
        // t = (P(t) - origin) / dir
        glm::vec3 tmin((boxMin.x - position.x) / front.x, (boxMin.y - position.y) / front.y, (boxMin.z - position.z) / front.z);
        glm::vec3 tmax((boxMax.x - position.x) / front.x, (boxMax.y - position.y) / front.y, (boxMax.z - position.z) / front.z);
        if (tmin.x > tmax.x) std::swap(tmin.x, tmax.x);
        if (tmin.y > tmax.y) std::swap(tmin.y, tmax.y);
        if (tmin.z > tmax.z) std::swap(tmin.z, tmax.z);
        if (tmin.x > tmax.y || tmin.y > tmax.x) return false;
        float mergedtmin = std::max(tmin.x, tmin.y);
        float mergedtmax = std::min(tmax.x, tmax.y);
        if (mergedtmin > tmax.z || tmin.z > mergedtmax) return false;
        mergedtmin = std::max(mergedtmin, tmin.z);
        mergedtmax = std::min(mergedtmax, tmax.z);
        dist = mergedtmin;
        // if ray entry is behind the camera, try ray exit.
        if (dist < 0) {
            dist = mergedtmax;
            if (dist < 0) return false;
        }
        return true;
    }

    void processKeyboard(movementDirection direction, float deltaTime) {
        if (direction == FORWARD)
            position += frontHorizontal * speed * deltaTime;
        if (direction == BACKWARD)
            position -= frontHorizontal * speed * deltaTime;
        if (direction == LEFT)
            position -= right * speed * deltaTime;
        if (direction == RIGHT)
            position += right * speed * deltaTime;
        if (direction == UP)
            position += worldUp * speed * 0.8f * deltaTime;
        if (direction == DOWN)
            position -= worldUp * speed * 0.8f * deltaTime;
    }

    void processMouseMovement(float xOffset, float yOffset) {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(front);
        frontHorizontal = glm::vec3(cos(glm::radians(yaw)), 0.0f, sin(glm::radians(yaw)));
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

