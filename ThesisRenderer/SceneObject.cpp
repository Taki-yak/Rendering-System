#include "SceneObject.h"
#include <glm/gtc/type_ptr.hpp>
#include "Renderer.h"
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

    renderer.DrawMesh(*mesh, *shader, model);

    for (auto child : children)
    {
        child->Draw(renderer, model);
    }
}