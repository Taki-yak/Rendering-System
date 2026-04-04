#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void Renderer::DrawMesh(Mesh& mesh, Shader& shader, glm::mat4 model)
{
    shader.use();
    shader.setMat4("model", glm::value_ptr(model));
    mesh.Draw();
}
void Renderer::Render(Scene& scene, Camera& camera)
{
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );

    for (auto obj : scene.objects)
    {
        Shader* shader = obj->shader;
        shader->use();

        shader->setMat4("view", glm::value_ptr(view));
        shader->setMat4("projection", glm::value_ptr(projection));
        shader->setVec3("viewPos", camera.Position);

        glm::mat4 model = obj->transform.GetModelMatrix();
        shader->setMat4("model", glm::value_ptr(model));

        obj->material->Apply(*shader);

        obj->mesh->Draw();
    }
}