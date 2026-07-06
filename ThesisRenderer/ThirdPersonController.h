#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include "SceneObject.h"
#include "Camera.h"
#include <iostream>
#include "Scene.h"
enum class PlayerAnimState
{
    Idle,
    Walk,
    Run,
    Jump
};
class ThirdPersonController
{
    PlayerAnimState animState = PlayerAnimState::Idle;
public:
    float moveSpeed = 6.0f;
    float runSpeed = 12.0f;
    float animTime = 0.0f;
    float cameraDistance = 6.0f;
    float cameraHeight = 3.0f;
    float cameraSmoothSpeed = 8.0f;
    float rotationSmoothSpeed = 10.0f;
    float currentMoveSpeed = 0.0f;
    float acceleration = 18.0f;
    float deceleration = 22.0f;
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
    const char* GetAnimStateName()
    {
        switch (animState)
        {
        case PlayerAnimState::Idle:
            return "Idle";

        case PlayerAnimState::Walk:
            return "Walk";

        case PlayerAnimState::Run:
            return "Run";

        case PlayerAnimState::Jump:
            return "Jump";

        default:
            return "Unknown";
        }
    }
    void Update(
        GLFWwindow* window,
        SceneObject* player,
        Camera& camera,
        Scene& scene,
        float deltaTime
    )
    {



        if (player == nullptr)
            return;
        glm::vec3 oldPosition =
            player->transform.position;
        float smoothT =
            glm::clamp(
                cameraSmoothSpeed * deltaTime,
                0.0f,
                1.0f
            );
        float targetSpeed = moveSpeed;

        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            targetSpeed = runSpeed;
        }
        glm::vec3 cameraForward =
            glm::normalize(
                glm::vec3(
                    camera.Front.x,
                    0.0f,
                    camera.Front.z
                )
            );

        glm::vec3 cameraRight =
            glm::normalize(
                glm::cross(
                    cameraForward,
                    camera.Up
                )
            );

        glm::vec3 movement(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            movement += cameraForward;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            movement -= cameraForward;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            movement -= cameraRight;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            movement += cameraRight;
        if (glm::length(movement) > 0.0f)
        {
            currentMoveSpeed =
                glm::min(
                    currentMoveSpeed +
                    acceleration * deltaTime,
                    targetSpeed
                );
        }
        else
        {
            currentMoveSpeed =
                glm::max(
                    currentMoveSpeed -
                    deceleration * deltaTime,
                    0.0f
                );
        }
        if (glm::length(movement) > 0.0f)
        {
            movement = glm::normalize(movement);

            player->transform.position +=
                movement * currentMoveSpeed * deltaTime;
            // ================= ENVIRONMENT COLLISION =================

            float playerRadius =
                0.7f;

            for (SceneObject* obj : scene.objects)
            {
                if (obj == player)
                    continue;

                if (!obj->isCollider)
                    continue;

                glm::vec2 playerXZ =
                    glm::vec2(
                        player->transform.position.x,
                        player->transform.position.z
                    );

                glm::vec2 objectXZ =
                    glm::vec2(
                        obj->transform.position.x,
                        obj->transform.position.z
                    );

                float distance =
                    glm::distance(
                        playerXZ,
                        objectXZ
                    );

                float minDistance =
                    playerRadius +
                    obj->colliderRadius;

                if (distance < minDistance)
                {
                    player->transform.position.x =
                        oldPosition.x;

                    player->transform.position.z =
                        oldPosition.z;

                    break;
                }
            }
            float targetAngle =
                glm::degrees(
                    atan2(
                        movement.x,
                        movement.z
                    )
                );

            float currentAngle =
                player->transform.rotation.y;

            float angleDifference =
                targetAngle - currentAngle;

            if (angleDifference > 180.0f)
                angleDifference -= 360.0f;

            if (angleDifference < -180.0f)
                angleDifference += 360.0f;

            player->transform.rotation.y +=
                angleDifference *
                rotationSmoothSpeed *
                deltaTime;
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
        glm::vec3 targetCameraPosition =
            playerPos +
            glm::vec3(
                0.0f,
                cameraHeight,
                0.0f
            ) +
            offset;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            camera.Position = targetCameraPosition;
        }
        else
        {
            camera.Position =
                glm::mix(
                    camera.Position,
                    targetCameraPosition,
                    smoothT
                );
        }

        glm::vec3 targetFront =
            glm::normalize(
                playerPos +
                glm::vec3(
                    0.0f,
                    1.2f,
                    0.0f
                )
                - camera.Position
            );

        camera.Front = targetFront;
        bool isMoving =
            glm::length(movement) > 0.0f;

        bool isRunning =
            glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        if (!isGrounded)
        {
            animState = PlayerAnimState::Jump;
        }
        else if (isMoving && isRunning)
        {
            animState = PlayerAnimState::Run;
        }
        else if (isMoving)
        {
            animState = PlayerAnimState::Walk;
        }
        else
        {
            animState = PlayerAnimState::Idle;
        }
        static PlayerAnimState lastState =
            PlayerAnimState::Idle;

        if (animState != lastState)
        {
            if (animState == PlayerAnimState::Idle)
                std::cout << "Animation: Idle\n";

            if (animState == PlayerAnimState::Walk)
                std::cout << "Animation: Walk\n";

            if (animState == PlayerAnimState::Run)
                std::cout << "Animation: Run\n";

            if (animState == PlayerAnimState::Jump)
                std::cout << "Animation: Jump\n";

            lastState = animState;
        }
      
    }
};