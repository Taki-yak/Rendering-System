#include "Scene.h"

void Scene::AddObject(SceneObject* object)
{
    objects.push_back(object);
}

void Scene::Render(Renderer& renderer)
{
    for (SceneObject* obj : objects)
    {
        renderer.DrawSceneObject(obj, glm::mat4(1.0f));
    }
}