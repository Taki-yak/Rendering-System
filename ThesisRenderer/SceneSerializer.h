#pragma once
#include "Scene.h"
#include <fstream>

class SceneSerializer
{
public:
    static void Save(Scene& scene, const std::string& filename);
    static void Load(Scene& scene, const std::string& filename,
        Mesh* mesh, Shader* shader, Material* material);
};