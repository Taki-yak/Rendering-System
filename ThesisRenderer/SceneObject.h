#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Transform.h"

class SceneObject
{
public:

    Mesh* mesh;
    Shader* shader;
    Material* material;

    Transform transform;

    SceneObject(Mesh* m, Shader* s, Material* mat);
};