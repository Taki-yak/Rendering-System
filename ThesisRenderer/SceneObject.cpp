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

void SceneObject::Draw(Renderer& renderer, glm::mat4 parentTransform)
{
    glm::mat4 model = parentTransform * transform.GetModelMatrix();

    shader->use();
    shader->setMat4("model", glm::value_ptr(model));
    shader->setVec3("materialAmbient", material->ambient);
    shader->setVec3("materialDiffuse", material->diffuseColor);
    shader->setVec3("materialSpecular", material->specular);
    shader->setFloat("materialShininess", material->shininess);

    mesh->Draw();

    for (SceneObject* child : children)
    {
        child->Draw(renderer, model);
    }
}