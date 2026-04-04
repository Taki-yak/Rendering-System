#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>

class SceneObject;
class Scene;
class  Camera;
class Renderer
{
public:
    void DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model);
    void Render(Scene& scene, Camera& camera);
}; 