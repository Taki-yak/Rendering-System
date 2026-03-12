#pragma once
#include <glad/glad.h>

class Mesh
{
public:

    unsigned int VAO, VBO;
    int vertexCount;

    Mesh(float* vertices, int size);

    void Draw();
};