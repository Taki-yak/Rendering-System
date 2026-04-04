#include <vector>
#include "SceneObject.h"

class Scene
{
public:
    std::vector<SceneObject*> objects;

    void AddObject(SceneObject* obj)
    {
        objects.push_back(obj);
    }
};