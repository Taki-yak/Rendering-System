#pragma once
#include "Texture.h"
#include "Shader.h"
#include <glm/glm.hpp> 

class Material
{
public:
    Texture* texture;

    glm::vec3 ambient = glm::vec3(0.1f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 diffuseColor = glm::vec3(1.0f); 
    glm::vec3 specular = glm::vec3(0.5f);
    float shininess = 32.0f;

    Material(Texture* tex)
    {
        texture = tex;
    }

    void Apply(Shader& shader)
    {
        texture->Bind();
        shader.setInt("texture1", 0);
    }
};