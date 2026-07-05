#include "AnimatedModel.h"

AnimatedModel::AnimatedModel(const std::string& path)
{
    Load(path);
}

void AnimatedModel::Load(const std::string& path)
{
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs
        );

    if (!scene || !scene->mRootNode)
    {
        std::cout << "Animation load failed: "
            << importer.GetErrorString()
            << std::endl;

        return;
    }

    std::cout << "Animated model loaded: "
        << path
        << std::endl;

    std::cout << "Meshes: "
        << scene->mNumMeshes
        << std::endl;

    std::cout << "Animations: "
        << scene->mNumAnimations
        << std::endl;

    for (unsigned int i = 0; i < scene->mNumAnimations; i++)
    {
        aiAnimation* animation =
            scene->mAnimations[i];

        std::cout << "Animation "
            << i
            << ": "
            << animation->mName.C_Str()
            << std::endl;

        std::cout << "Duration: "
            << animation->mDuration
            << std::endl;

        std::cout << "Ticks Per Second: "
            << animation->mTicksPerSecond
            << std::endl;

        std::cout << "Channels/Bones: "
            << animation->mNumChannels
            << std::endl;
    }
}