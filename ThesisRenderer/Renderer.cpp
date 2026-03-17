#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>

void Renderer::DrawObject(SceneObject* object)
{
    object->shader->use();

    glm::mat4 model = object->transform.GetModelMatrix();

    object->shader->setMat4("model", glm::value_ptr(model));

    if (object->material)
        object->material->Bind();

    object->mesh->Draw();
}

void Renderer::DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model)
{
    shader.use();
    shader.setMat4("model", glm::value_ptr(model));
    mesh.Draw();
}