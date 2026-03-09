#pragma once

#include <glad/glad.h>
#include <string>

class Texture
{
public:

    unsigned int ID;

    Texture(const char* path);

    void Bind();
};