#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>

class SceneObject;

class Renderer
{
public:
    void DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model);
};