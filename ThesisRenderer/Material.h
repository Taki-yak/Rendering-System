#pragma once

#include <glm/glm.hpp>

class Texture;

class Material
{
public:

    Texture* texture;

    glm::vec3 ambient =
        glm::vec3(0.3f);

    glm::vec3 diffuse =
        glm::vec3(1.0f);

    glm::vec3 specular =
        glm::vec3(0.5f);
    glm::vec3 tint =
        glm::vec3(1.0f);
    float shininess = 32.0f;

    bool wireframe = false;

    Material(Texture* tex);
};
