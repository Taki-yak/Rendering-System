#include "Scene.h"

void Scene::AddObject(SceneObject* obj)
{
    objects.push_back(obj);
}

void Scene::Render(Renderer& renderer)
{
    for (SceneObject* obj : objects)
    {
        renderer.DrawObject(*obj);
    }
}