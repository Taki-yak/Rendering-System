#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "SceneObject.h"

class Renderer
{
public:

    void DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model);
    void DrawObject(SceneObject* object);
};