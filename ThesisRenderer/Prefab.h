#pragma once

#include <string>
#include <glm/glm.hpp>

struct Prefab
{
    std::string name;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};