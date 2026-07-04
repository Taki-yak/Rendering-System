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

        camera.Position =
            glm::vec3(
                playerPos.x,
                playerPos.y + cameraHeight,
                playerPos.z + cameraDistance
            );

        camera.Front =
            glm::normalize(
                playerPos -
                camera.Position
            );
    }
};