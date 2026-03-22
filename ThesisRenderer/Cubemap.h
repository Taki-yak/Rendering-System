#pragma once
#include <glad/glad.h>
#include <vector>
#include <string>

class Cubemap
{
public:

    unsigned int ID;

    Cubemap(const std::vector<std::string>& faces);

    void Bind();
};