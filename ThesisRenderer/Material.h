#pragma once

#include "Texture.h"
#include <glm/glm.hpp>

class Material
{
public:

    Texture* diffuse;
    glm::vec3 specular;
    float shininess;

    Material(Texture* tex)
    {
        diffuse = tex;
        specular = glm::vec3(0.5f);
        shininess = 32.0f;
    }

    void Bind()
    {
        if (diffuse)
            diffuse->Bind();
    }
};