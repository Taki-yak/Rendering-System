#pragma once
#include "Component.h"

class RotateComponent : public Component
{
public:

    float speed = 50.0f;

    void Update(float deltaTime) override;
};