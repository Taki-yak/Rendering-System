#include "SceneObject.h"
#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>

SceneObject::SceneObject(Mesh* m, Shader* s, Material* mat)
{
    mesh = m;
    shader = s;
    material = mat;
}


void SceneObject::AddChild(SceneObject* child)
{
    child->parent = this;
    children.push_back(child);
}
SceneObject::SceneObject(Model* model, Shader* s)
{
    this->model = model;
    this->shader = s;

    this->mesh = nullptr;
    this->material = nullptr;

    this->useModel = true;
}
void SceneObject::Draw(Renderer& renderer, glm::mat4 parentTransform)
{
    if (!visible)
        return;

    glm::mat4 modelMatrix =
        parentTransform *
        transform.GetModelMatrix();

    shader->use();

    shader->setMat4(
        "model",
        glm::value_ptr(modelMatrix)
    );

    shader->setBool(
        "isSelected",
        isSelected
    );

    if (useModel && model != nullptr)
    {
        model->Draw();
    }
    else if (mesh != nullptr && material != nullptr)
    {
        shader->setVec3(
            "materialAmbient",
            material->ambient
        );

        shader->setVec3(
            "materialDiffuse",
            material->diffuse
        );

        shader->setVec3(
            "materialSpecular",
            material->specular
        );

        shader->setVec3(
            "materialTint",
            material->tint
        );

        shader->setFloat(
            "materialShininess",
            material->shininess
        );

        mesh->Draw();
    }

    for (SceneObject* child : children)
    {
        child->Draw(
            renderer,
            modelMatrix
        );
    }
}