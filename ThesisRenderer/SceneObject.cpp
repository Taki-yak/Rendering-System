#include "SceneObject.h"

SceneObject::SceneObject(Mesh* m, Shader* s, Texture* t)
{
    mesh = m;
    shader = s;
    texture = t;
}