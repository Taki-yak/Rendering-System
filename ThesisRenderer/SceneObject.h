#pragma once

#include <vector>
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Transform.h"
#include "Component.h"
#include "Model.h"
#include "Light.h"
#include <string>
class Renderer;
enum class AssetType
{
    Unknown,
    Terrain,
    Player,
    Tree,
    Rock,
    Grass,
    Flower,
    Bush,
    House,
    Mountain,
    Fence,
    Prop,
    Light,
    Gameplay
};

enum class SpawnSource
{
    Manual,
    Procedural,
    LoadedScene
};
class SceneObject
{
public:
    std::string name = "GameObject";
    std::string assetId =
        "unknown";

    AssetType assetType =
        AssetType::Unknown;

    SpawnSource spawnSource =
        SpawnSource::Manual;

    bool persistent =
        true;

    bool showInHierarchy =
        true;
    SceneObject(Mesh* m, Shader* s, Material* mat);
    ~SceneObject()
    {
        for (Component* c : components)
            delete c;
    }

    Transform transform;
    bool visible = true;
    bool isSelected = false;
    bool isCollider = false;
    float colliderRadius = 1.0f;
    void AddChild(SceneObject* child);
    void Draw(Renderer& renderer, glm::mat4 parentTransform);
    Light* attachedLight =
        nullptr;

    glm::vec3 attachedLightOffset =
        glm::vec3(
            0.0f
        );
    
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
    Model* model = nullptr;
    bool useModel = false;
    SceneObject(Model* model, Shader* shader);
    float boundingRadius = 1.0f;
    SceneObject* parent = nullptr;
    std::vector<SceneObject*> children;
    std::vector<Component*> components;
  
};