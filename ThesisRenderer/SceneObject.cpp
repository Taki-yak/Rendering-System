#include "SceneObject.h"

SceneObject::SceneObject(Mesh* m, Shader* s, Material* mat)
{
    mesh = m;
    shader = s;
    material = mat;
}