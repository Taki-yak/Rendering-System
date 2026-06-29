#include "AssetDatabase.h"

std::vector<AssetEntry>
AssetDatabase::assets;

void AssetDatabase::Initialize()
{
    assets.clear();

    assets.push_back(
        {
            "Tree",
            "Assets/Models/Tree.obj",
            "Model"
        });

    assets.push_back(
        {
            "Rock",
            "Assets/Models/Rock.obj",
            "Model"
        });

    assets.push_back(
        {
            "House",
            "Assets/Models/House.obj",
            "Model"
        });

    assets.push_back(
        {
            "Fence",
            "Assets/Models/Fence.obj",
            "Model"
        });

    assets.push_back(
        {
            "Barrel",
            "Assets/Models/Barrel.obj",
            "Model"
        });

    assets.push_back(
        {
            "Character",
            "Assets/Models/Character.obj",
            "Model"
        });
}