#include "GridRenderer.h"
#include <vector>

GridRenderer::GridRenderer(int gridSize, float spacing)
{
    std::vector<float> vertices;

    for (int i = -gridSize; i <= gridSize; i++)
    {
        // Lines parallel to Z
        vertices.push_back(i * spacing);
        vertices.push_back(0.0f);
        vertices.push_back(-gridSize * spacing);

        vertices.push_back(i * spacing);
        vertices.push_back(0.0f);
        vertices.push_back(gridSize * spacing);

        // Lines parallel to X
        vertices.push_back(-gridSize * spacing);
        vertices.push_back(0.0f);
        vertices.push_back(i * spacing);

        vertices.push_back(gridSize * spacing);
        vertices.push_back(0.0f);
        vertices.push_back(i * spacing);
    }

    vertexCount = vertices.size() / 3;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void GridRenderer::Draw(Shader& shader)
{
    shader.use();

    glBindVertexArray(VAO);

    glDrawArrays(GL_LINES, 0, vertexCount);

    glBindVertexArray(0);
}