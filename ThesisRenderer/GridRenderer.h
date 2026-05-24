#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class GridRenderer
{
public:

    GridRenderer(int gridSize = 20, float spacing = 1.0f);

    void Draw(Shader& shader);

private:

    unsigned int VAO, VBO;

    int vertexCount;
};