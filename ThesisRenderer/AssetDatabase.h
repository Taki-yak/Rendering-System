#pragma once

#include <string>
#include <vector>

struct AssetEntry
{
    std::string name;

    std::string path;

    std::string type;
};

class AssetDatabase
{
public:

    static std::vector<AssetEntry> assets;

    static void Initialize();
};