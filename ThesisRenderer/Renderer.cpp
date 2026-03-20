#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>

void Renderer::DrawObject(SceneObject* object)
{
    object->shader->use();

    glm::mat4 model = object->transform.GetModelMatrix();

    object->shader->setMat4("model", glm::value_ptr(model));

    if (object->material && object->material->diffuse)
    {
        object->material->diffuse->Bind();
    }

    object->shader->setVec3("materialAmbient", object->material->ambient);
    object->shader->setVec3("materialDiffuse", object->material->diffuseColor);
    object->shader->setVec3("materialSpecular", object->material->specular);
    object->shader->setFloat("materialShininess", object->material->shininess);

    object->mesh->Draw();
}

void Renderer::DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model)
{
    shader.use();
    shader.setMat4("model", glm::value_ptr(model));
    mesh.Draw();
}
void Renderer::DrawSceneObject(SceneObject* object, glm::mat4 parentMatrix)
{
    glm::mat4 model = parentMatrix * object->transform.GetModelMatrix();

    object->shader->use();
    object->shader->setMat4("model", glm::value_ptr(model));

    if (object->material && object->material->diffuse)
        object->material->diffuse->Bind();

    object->shader->setVec3("materialAmbient", object->material->ambient);
    object->shader->setVec3("materialDiffuse", object->material->diffuseColor);
    object->shader->setVec3("materialSpecular", object->material->specular);
    object->shader->setFloat("materialShininess", object->material->shininess);

    object->mesh->Draw();

    for (SceneObject* child : object->children)
    {
        DrawSceneObject(child, model);
    }
}