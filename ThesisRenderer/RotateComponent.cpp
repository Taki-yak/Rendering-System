#include "RotateComponent.h"
#include "SceneObject.h"

void RotateComponent::Update(float deltaTime)
{
    owner->transform.rotation.y +=
        speed * deltaTime;
}