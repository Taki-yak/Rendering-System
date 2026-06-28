#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

class PlayerController
{
public:
    float moveSpeed = 6.0f;
    float jumpForce = 6.5f;
    float gravity = -18.0f;

    float verticalVelocity = 0.0f;

    float playerHeight = 1.8f;
    float groundY = 0.0f;

    bool isGrounded = true;

    void Update(
        GLFWwindow* window,
        Camera& camera,
        float deltaTime
    )
    {
        glm::vec3 forward =
            glm::normalize(
                glm::vec3(
                    camera.Front.x,
                    0.0f,
                    camera.Front.z
                )
            );

        glm::vec3 right =
            glm::normalize(
                glm::cross(
                    forward,
                    camera.Up
                )
            );

        glm::vec3 movement =
            glm::vec3(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement += forward;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement -= forward;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement -= right;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement += right;

        if (glm::length(movement) > 0.0f)
        {
            movement =
                glm::normalize(movement);
        }

        camera.Position +=
            movement *
            moveSpeed *
            deltaTime;

        if (
            isGrounded &&
            glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS
            )
        {
            verticalVelocity = jumpForce;
            isGrounded = false;
        }

        verticalVelocity +=
            gravity *
            deltaTime;

        camera.Position.y +=
            verticalVelocity *
            deltaTime;

        float minimumY =
            groundY + playerHeight;

        if (camera.Position.y <= minimumY)
        {
            camera.Position.y = minimumY;
            verticalVelocity = 0.0f;
            isGrounded = true;
        }
    }
};