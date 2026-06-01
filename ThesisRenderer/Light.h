#pragma once

#include <glm/glm.hpp>
#include <string>

class Light
{
public:

    std::string name;

    glm::vec3 position;

    glm::vec3 color;

    float intensity;

    Light()
    {
        name = "Light";

        position = glm::vec3(0.0f);

        color = glm::vec3(1.0f);

        intensity = 1.0f;
    }
};