#include "Scene.h"

void Scene::AddObject(SceneObject* obj)
{
    objects.push_back(obj);
}

void Scene::Render(Renderer& renderer)
{ 
 
    for (auto obj : objects)
    {

        obj->Draw(renderer, glm::mat4(1.0f));
    }
}
