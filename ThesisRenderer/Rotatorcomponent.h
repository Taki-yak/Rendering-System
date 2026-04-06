#pragma once

#include "Component.h"
#include "SceneObject.h"
#include <glm/glm.hpp>

class RotatorComponent : public Component
{
public:
    glm::vec3 axis;       
    float degreesPerSec;  

    RotatorComponent(glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f),
        float degreesPerSec = 45.0f)
        : axis(axis), degreesPerSec(degreesPerSec)
    {}

    void Update(float deltaTime) override
    {
        owner->transform.rotation += axis * degreesPerSec * deltaTime;
    }
};
