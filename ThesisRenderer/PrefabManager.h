#pragma once

#include <vector>
#include "Prefab.h"

class PrefabManager
{
public:

    static std::vector<Prefab> prefabs;

    static void SavePrefab(
        const Prefab& prefab
    );
};