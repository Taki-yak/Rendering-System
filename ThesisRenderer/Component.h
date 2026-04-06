#pragma once

class SceneObject;

class Component
{
public:
    SceneObject* owner = nullptr;

    virtual ~Component() = default;

    virtual void Update(float deltaTime) {}

    virtual void OnAttach() {}
};
