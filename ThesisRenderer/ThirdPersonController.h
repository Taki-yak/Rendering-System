#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SceneObject.h"
#include "Camera.h"

class ThirdPersonController
{
public:
    float moveSpeed = 6.0f;
    float runSpeed = 12.0f;

    float cameraDistance = 6.0f;
    float cameraHeight = 3.0f;
    float gravity = -18.0f;
    float jumpForce = 7.5f;
    float verticalVelocity = 0.0f;
    float yaw = 180.0f;
    float pitch = 20.0f;

    float mouseSensitivity = 0.15f;
    float lastMouseX = 0.0f;
    float lastMouseY = 0.0f;
    bool firstMouse = true;
    float groundY = 0.0f;
    bool isGrounded = true;
    void Update(
        GLFWwindow* window,
        SceneObject* player,
        Camera& camera,
        float deltaTime
    )
    {
        if (player == nullptr)
            return;

        float currentSpeed = moveSpeed;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            currentSpeed = runSpeed;
        }

        glm::vec3 movement(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement.z -= 1.0f;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement.z += 1.0f;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement.x -= 1.0f;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement.x += 1.0f;

        if (glm::length(movement) > 0.0f)
        {
            movement = glm::normalize(movement);

            player->transform.position +=
                movement * currentSpeed * deltaTime;

            float angle =
                atan2(movement.x, movement.z);

            player->transform.rotation.y =
                glm::degrees(angle);
        }
        if (
            isGrounded &&
            glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS
            )
        {
            verticalVelocity = jumpForce;
            isGrounded = false;
        }

        verticalVelocity +=
            gravity * deltaTime;

        player->transform.position.y +=
            verticalVelocity * deltaTime;

        if (player->transform.position.y <= groundY)
        {
            player->transform.position.y = groundY;
            verticalVelocity = 0.0f;
            isGrounded = true;
        }

        glm::vec3 playerPos =
            player->transform.position;

        double mouseX;
        double mouseY;

        glfwGetCursorPos(
            window,
            &mouseX,
            &mouseY
        );

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            if (firstMouse)
            {
                lastMouseX = (float)mouseX;
                lastMouseY = (float)mouseY;
                firstMouse = false;
            }

            float xOffset =
                ((float)mouseX - lastMouseX) *
                mouseSensitivity;
            float yOffset =
                (lastMouseY - (float)mouseY) *
                mouseSensitivity;

            lastMouseX = (float)mouseX;
            lastMouseY = (float)mouseY;
            yaw -= xOffset;
            pitch -= yOffset;

            if (pitch > 60.0f)
                pitch = 60.0f;

            if (pitch < -10.0f)
                pitch = -10.0f;
        }
        else
        {
            firstMouse = true;
        }

        float yawRad =
            glm::radians(yaw);

        float pitchRad =
            glm::radians(pitch);

        glm::vec3 offset;

        offset.x =
            cameraDistance *
            cos(pitchRad) *
            sin(yawRad);

        offset.y =
            cameraDistance *
            sin(pitchRad);

        offset.z =
            cameraDistance *
            cos(pitchRad) *
            cos(yawRad);

        camera.Position =
            playerPos +
            glm::vec3(
                0.0f,
                cameraHeight,
                0.0f
            ) +
            offset;

        camera.Front =
            glm::normalize(
                playerPos +
                glm::vec3(0.0f, 1.2f, 0.0f)
                - camera.Position
            );
    }
};