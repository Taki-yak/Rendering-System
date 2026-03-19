#pragma once
#include <glm/glm.hpp>
#include "Texture.h"

class Material
{
public:

    Texture* diffuse;

    glm::vec3 ambient;
    glm::vec3 diffuseColor;
    glm::vec3 specular;

    float shininess;

    Material(Texture* tex)
    {
        diffuse = tex;

        ambient = glm::vec3(0.2f);
        diffuseColor = glm::vec3(1.0f);
        specular = glm::vec3(0.5f);
        shininess = 32.0f;
    }
};