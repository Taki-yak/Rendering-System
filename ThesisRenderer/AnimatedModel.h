#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "AnimatedVertex.h"
#include "BoneInfo.h"
#include <glm/gtc/type_ptr.hpp>
class AnimatedModel
{
public:
    AnimatedModel(const std::string& path);

    void Load(const std::string& path);

    int GetBoneCount() const
    {
        return boneCounter;
    }

private:
    std::vector<AnimatedVertex> vertices;

    std::map<std::string, BoneInfo> boneInfoMap;

    int boneCounter = 0;

    void ProcessNode(aiNode* node, const aiScene* scene);

    void ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void ExtractBoneWeightForVertices(
        aiMesh* mesh,
        const aiScene* scene
    );

    void SetVertexBoneData(
        AnimatedVertex& vertex,
        int boneID,
        float weight
    );
};