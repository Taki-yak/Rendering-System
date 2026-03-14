#pragma once

#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
class SceneObject
{
public:

    Mesh* mesh;
    Shader* shader;
    Texture* texture;

    Transform transform;

    SceneObject(Mesh* m, Shader* s, Texture* t)
        : mesh(m), shader(s), texture(t) {}
};