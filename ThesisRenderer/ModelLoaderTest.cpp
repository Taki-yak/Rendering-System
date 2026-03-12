#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

void TestAssimp()
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(
        "model.obj",
        aiProcess_Triangulate | aiProcess_FlipUVs
    );

    if (!scene)
    {
        std::cout << "Assimp error: "
            << importer.GetErrorString()
            << std::endl;
        return;
    }

    std::cout << "Model loaded successfully!" << std::endl;
}