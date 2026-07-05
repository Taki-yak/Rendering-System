#pragma once

#include <string>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AnimatedModel
{
public:
    AnimatedModel(const std::string& path);

    void Load(const std::string& path);
};