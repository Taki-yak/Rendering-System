#pragma once

#include <vector>
#include <string>

#include "Mesh.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Model
{
public:

    Model(const std::string& path);
    void Draw();

private:
    std::vector<Mesh> meshes;
    std::vector<Texture> loadedTextures;
    std::string directory;

    void LoadModel(std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};