#include "AnimatedModel.h"
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <windows.h>
AnimatedModel::AnimatedModel(const std::string& path)
{
    Load(path);
}

void AnimatedModel::Load(const std::string& path)
{
    char currentDir[MAX_PATH];

    GetCurrentDirectoryA(
        MAX_PATH,
        currentDir
    );

    std::cout
        << "Current working directory: "
        << currentDir
        << std::endl;

    std::cout
        << "Trying to load FBX: "
        << path
        << std::endl;
    std::ifstream fileCheck(path);

    if (!fileCheck.good())
    {
        std::cout
            << "FBX FILE NOT FOUND: "
            << path
            << std::endl;

        return;
    }
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs |
            aiProcess_LimitBoneWeights
        );

    if (!scene || !scene->mRootNode)
    {
        std::cout
            << "Animated model load failed: "
            << importer.GetErrorString()
            << std::endl;

        return;
    }

    std::cout
        << "Animated model loaded: "
        << path
        << std::endl;

    std::cout
        << "Meshes: "
        << scene->mNumMeshes
        << std::endl;

    std::cout
        << "Animations: "
        << scene->mNumAnimations
        << std::endl;

    ProcessNode(
        scene->mRootNode,
        scene
    );

    std::cout
        << "Total animated vertices: "
        << vertices.size()
        << std::endl;

    std::cout
        << "Total bones found: "
        << boneCounter
        << std::endl;

    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* animation =
            scene->mAnimations[i];

        std::cout
            << "Animation "
            << i
            << ": "
            << animation->mName.C_Str()
            << std::endl;

        std::cout
            << "Duration: "
            << animation->mDuration
            << std::endl;

        std::cout
            << "Ticks Per Second: "
            << animation->mTicksPerSecond
            << std::endl;

        std::cout
            << "Channels: "
            << animation->mNumChannels
            << std::endl;
    }
}

void AnimatedModel::ProcessNode(
    aiNode* node,
    const aiScene* scene
)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh =
            scene->mMeshes[node->mMeshes[i]];

        ProcessMesh(
            mesh,
            scene
        );
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(
            node->mChildren[i],
            scene
        );
    }
}

void AnimatedModel::ProcessMesh(
    aiMesh* mesh,
    const aiScene* scene
)
{
    unsigned int vertexStartIndex =
        vertices.size();

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        AnimatedVertex vertex;

        vertex.position =
            glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );

        if (mesh->HasNormals())
        {
            vertex.normal =
                glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                );
        }
        else
        {
            vertex.normal =
                glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->mTextureCoords[0])
        {
            vertex.texCoords =
                glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
        }
        else
        {
            vertex.texCoords =
                glm::vec2(0.0f);
        }

        vertices.push_back(vertex);
    }

    ExtractBoneWeightForVertices(
        mesh,
        scene
    );

    std::cout
        << "Processed mesh vertices: "
        << mesh->mNumVertices
        << " starting at "
        << vertexStartIndex
        << std::endl;
}

void AnimatedModel::ExtractBoneWeightForVertices(
    aiMesh* mesh,
    const aiScene* scene
)
{
    for (unsigned int boneIndex = 0;
        boneIndex < mesh->mNumBones;
        boneIndex++)
    {
        int boneID = -1;

        std::string boneName =
            mesh->mBones[boneIndex]->mName.C_Str();

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;

            newBoneInfo.id =
                boneCounter;

            aiMatrix4x4 aiOffset =
                mesh->mBones[boneIndex]->mOffsetMatrix;

            newBoneInfo.offset =
                glm::transpose(
                    glm::make_mat4(
                        &aiOffset.a1
                    )
                );

            boneInfoMap[boneName] =
                newBoneInfo;

            boneID =
                boneCounter;

            boneCounter++;

            std::cout
                << "New bone: "
                << boneName
                << " ID: "
                << boneID
                << std::endl;
        }
        else
        {
            boneID =
                boneInfoMap[boneName].id;
        }

        aiVertexWeight* weights =
            mesh->mBones[boneIndex]->mWeights;

        int numWeights =
            mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0;
            weightIndex < numWeights;
            weightIndex++)
        {
            int vertexID =
                weights[weightIndex].mVertexId;

            float weight =
                weights[weightIndex].mWeight;

            if (vertexID < vertices.size())
            {
                SetVertexBoneData(
                    vertices[vertexID],
                    boneID,
                    weight
                );
            }
        }
    }
}

void AnimatedModel::SetVertexBoneData(
    AnimatedVertex& vertex,
    int boneID,
    float weight
)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (vertex.boneIDs[i] < 0)
        {
            vertex.boneIDs[i] =
                boneID;

            vertex.weights[i] =
                weight;

            return;
        }
    }
}