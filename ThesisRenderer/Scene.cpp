#include "Scene.h"

void Scene::AddObject(SceneObject* object)
{
    objects.push_back(object);
}

void Scene::Render(Renderer& renderer)
{
    for (auto obj : objects)
    {
        obj->Draw(renderer, glm::mat4(1.0f));
    }
}