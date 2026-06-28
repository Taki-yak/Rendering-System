#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Camera.h"

class PlayerController
{
public:
    float moveSpeed = 6.0f;

    void Update(
        GLFWwindow* window,
        Camera& camera,
        float deltaTime
    )
    {
        float velocity =
            moveSpeed * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.Position += velocity * camera.Front;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.Position -= velocity * camera.Front;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.Position -=
            glm::normalize(
                glm::cross(
                    camera.Front,
                    camera.Up
                )
            ) * velocity;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.Position +=
            glm::normalize(
                glm::cross(
                    camera.Front,
                    camera.Up
                )
            ) * velocity;
    }
};