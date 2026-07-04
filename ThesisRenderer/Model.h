#pragma once

#include <vector>
#include <string>

#include "Mesh.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include "Shader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
    Model(const std::string& path, const std::string& textureFolder = "");

    void Draw(Shader& shader);

private:
    std::vector<Mesh> meshes;
    std::vector<Texture> loadedTextures;

    std::string directory;
    std::string textureDirectory;
    std::vector<glm::vec3> meshDiffuseColors;
    void LoadModel(std::string path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
};