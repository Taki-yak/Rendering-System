#pragma once

#include <glm/glm.hpp>
#include <string>

enum class LightType
{
    Point,
    Directional
};

class Light
{
public:

    std::string name;

    glm::vec3 position;

    glm::vec3 color;

    float intensity;

   
    LightType type;

    glm::vec3 direction;

    Light()
    {
        name = "Light";

        position = glm::vec3(0.0f);

        color = glm::vec3(1.0f);

        intensity = 1.0f;

       
        type = LightType::Point;

        direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    }
};