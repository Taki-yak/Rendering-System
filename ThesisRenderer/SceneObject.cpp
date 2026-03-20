#include "SceneObject.h"

SceneObject::SceneObject(Mesh* m, Shader* s, Material* mat)
{
    mesh = m;
    shader = s;
    material = mat;
}
void SceneObject::AddChild(SceneObject* child)
{
    child->parent = this;
    children.push_back(child);
}