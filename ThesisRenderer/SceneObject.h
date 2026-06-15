#pragma once

#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Transform.h"
#include "Component.h"

class Renderer;

class SceneObject
{
public:
    std::string name = "GameObject";
    SceneObject(Mesh* m, Shader* s, Material* mat);
    ~SceneObject()
    {
        for (Component* c : components)
            delete c;
    }

    Transform transform;
    bool isSelected = false;
    void AddChild(SceneObject* child);
    void Draw(Renderer& renderer, glm::mat4 parentTransform);

    
    void AddComponent(Component* component)
    {
        component->owner = this;
        component->OnAttach();
        components.push_back(component);
    }

    template<typename T>
    T* GetComponent()
    {
        for (Component* c : components)
        {
            T* result = dynamic_cast<T*>(c);
            if (result) return result;
        }
        return nullptr;
    }

    void UpdateComponents(float deltaTime)
    {
        for (Component* c : components)
            c->Update(deltaTime);
        for (SceneObject* child : children)
            child->UpdateComponents(deltaTime);
    }
   
public:
    Mesh* mesh;
    Shader* shader;
    Material* material;
    float boundingRadius = 1.0f;
    SceneObject* parent = nullptr;
    std::vector<SceneObject*> children;
    std::vector<Component*> components;
  
};