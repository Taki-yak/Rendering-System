#pragma once
#include <vector>
#include "SceneObject.h"
#include "Renderer.h"
#include "Light.h"
class Scene
{
public:
    std::vector<SceneObject*> objects;
    std::vector<Light*> lights;
    void AddObject(SceneObject* obj);

    void AddLight(Light* light);

    void Render(Renderer& renderer);
   
};