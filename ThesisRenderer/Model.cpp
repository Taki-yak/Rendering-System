#include "Model.h"
#include <iostream>

Model::Model(const std::string& path)
{
    LoadModel(path);
}
void Model::Draw()
{
    if (!loadedTextures.empty())
    {
        glActiveTexture(GL_TEXTURE0);
        loadedTextures[0].Bind();
    }

    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Draw();
    }
}           

void Model::LoadModel(std::string path)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace
    );

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "Assimp error: " << importer.GetErrorString() << std::endl;
        return;
    }

    size_t slash = path.find_last_of("/\\");
    directory = (slash == std::string::npos) ? "." : path.substr(0, slash);

    ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}
Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<float> vertices;

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            unsigned int index = face.mIndices[j];

            vertices.push_back(mesh->mVertices[index].x);
            vertices.push_back(mesh->mVertices[index].y);
            vertices.push_back(mesh->mVertices[index].z);

            if (mesh->HasNormals())
            {
                vertices.push_back(mesh->mNormals[index].x);
                vertices.push_back(mesh->mNormals[index].y);
                vertices.push_back(mesh->mNormals[index].z);
            }
            else
            {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }

            if (mesh->mTextureCoords[0])
            {
                vertices.push_back(mesh->mTextureCoords[0][index].x);
                vertices.push_back(mesh->mTextureCoords[0][index].y);
            }
            else
            {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
    }


    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiString str;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS)
        {
            std::string filename = std::string(str.C_Str());

            size_t slash = filename.find_last_of("/\\");
            if (slash != std::string::npos)
            {
                filename = filename.substr(slash + 1);
            }

            std::string fullPath = directory + "/" + filename;

            std::cout << "Full texture path: " << fullPath << std::endl;

            std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
            std::cout << "Trying to load texture: " << fullPath << std::endl;

            Texture texture(fullPath.c_str());

            loadedTextures.push_back(texture);
        }
    }

    return Mesh(vertices.data(), vertices.size() * sizeof(float));
}