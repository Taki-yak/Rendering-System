#pragma once

#include <string>
#include <glm/glm.hpp>

struct Prefab
{
    std::string name = "Prefab";

    // Object identity
    std::string meshType = "";
    std::string modelPath = "";
    std::string modelDirectory = "";
    std::string texturePath = "";
    std::string gameplayType = "";

    // Transform
    glm::vec3 position =
        glm::vec3(0.0f);

    glm::vec3 rotation =
        glm::vec3(0.0f);

    glm::vec3 scale =
        glm::vec3(1.0f);

    // Object state
    bool isCollider =
        false;

    float colliderRadius =
        1.0f;
};