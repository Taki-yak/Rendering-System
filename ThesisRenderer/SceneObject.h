#pragma once

#include <glm/glm.hpp>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"

class SceneObject
{
public:

    Mesh* mesh;
    Shader* shader;
    Texture* texture;

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    SceneObject(Mesh* m, Shader* s, Texture* t);

    glm::mat4 GetModelMatrix();
};