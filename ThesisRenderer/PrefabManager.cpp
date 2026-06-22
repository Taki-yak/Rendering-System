#include "PrefabManager.h"

std::vector<Prefab>
PrefabManager::prefabs;

void PrefabManager::SavePrefab(
    const Prefab& prefab
)
{
    prefabs.push_back(prefab);
}