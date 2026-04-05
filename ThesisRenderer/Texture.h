#pragma once

#include <glad/glad.h>
#include <string>

class Texture
{
public:

    unsigned int ID = 0;

    Texture() = default;
    Texture(const char* path);

    void Bind();
};