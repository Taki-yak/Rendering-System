#pragma once

#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Transform.h"
#include "Renderer.h"

class SceneObject
{
public:
    SceneObject(Mesh* m, Shader* s, Material* mat);

    Transform transform;

    void AddChild(SceneObject* child);
   // void Draw(Renderer& renderer, glm::mat4 parentTransform);

public:
    Mesh* mesh;
    Shader* shader;
    Material* material;

    SceneObject* parent = nullptr;
    std::vector<SceneObject*> children;
};