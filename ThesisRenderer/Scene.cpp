#include "Scene.h"

void Scene::AddObject(SceneObject* object)
{
    objects.push_back(object);
}

void Scene::Render(Renderer& renderer)
{
    for (SceneObject* object : objects)
    {
        renderer.DrawObject(object);
    }
}