#pragma once
#include <vector>
#include "SceneObject.h"
#include "Renderer.h"

class Scene
{
public:
    std::vector<SceneObject*> objects;

    void AddObject(SceneObject* obj);
    void Render(Renderer& renderer);
};