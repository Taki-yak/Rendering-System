#include "EditorUI.h"
#include "Light.h"
#include <glm/gtc/type_ptr.hpp>
#include "SceneSerializer.h"
#include <algorithm>
#include "PrefabManager.h"
#include "AssetDatabase.h"
#include "Texture.h"
#include <unordered_map>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "mesh.h"
#include "EditorLayout.h"
#include <fstream>
#include <sstream>
float GetObjectTerrainY(
    float x,
    float z,
    float offset
);
extern float GetTerrainHeight(
    float x,
    float z
);
static glm::vec3 SnapEditorPositionToTerrain(
    glm::vec3 position,
    float offset = 0.05f
)
{
    position.y =
        GetTerrainHeight(
            position.x,
            position.z
        ) + offset;

    return position;
}
static void SetEditorSaveMetadata(
    SceneObject* object,
    const std::string& meshType,
    const std::string& gameplayType = "None",
    const std::string& modelPath = "",
    const std::string& modelDirectory = ""
)
{
    if (object == nullptr)
        return;

    object->editorMeshType =
        meshType;

    object->editorGameplayType =
        gameplayType;

    object->editorModelPath =
        modelPath;

    object->editorModelDirectory =
        modelDirectory;
}
static const char* GetAssetTypeName(
    AssetType assetType
)
{
    switch (assetType)
    {
    case AssetType::Terrain:
        return "Terrain";

    case AssetType::Player:
        return "Player";

    case AssetType::Tree:
        return "Tree";

    case AssetType::Rock:
        return "Rock";

    case AssetType::Grass:
        return "Grass";

    case AssetType::Flower:
        return "Flower";

    case AssetType::Bush:
        return "Bush";

    case AssetType::House:
        return "House";

    case AssetType::Mountain:
        return "Mountain";

    case AssetType::Fence:
        return "Fence";

    case AssetType::Prop:
        return "Prop";

    case AssetType::Light:
        return "Light";

    case AssetType::Gameplay:
        return "Gameplay";

    default:
        return "Unknown";
    }
}

static const char* GetSpawnSourceName(
    SpawnSource spawnSource
)
{
    switch (spawnSource)
    {
    case SpawnSource::Manual:
        return "Manual";

    case SpawnSource::Procedural:
        return "Procedural";

    case SpawnSource::LoadedScene:
        return "Loaded Scene";

    default:
        return "Unknown";
    }
}
static bool IsPrimitiveShapeObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorMeshType == "Sphere")
        return true;

    if (object->editorMeshType == "Cylinder")
        return true;

    if (object->editorMeshType == "Cone")
        return true;

    return false;
}

static void SetPrimitiveShapeScale(
    SceneObject* object,
    float x,
    float y,
    float z
)
{
    if (object == nullptr)
        return;

    if (x < 0.05f)
        x = 0.05f;

    if (y < 0.05f)
        y = 0.05f;

    if (z < 0.05f)
        z = 0.05f;

    object->transform.scale =
        glm::vec3(
            x,
            y,
            z
        );
}
static void DrawStairsDetailControls(
    SceneObject* selectedObject
);
static void ClampPrimitiveShapeScale(
    SceneObject* object
)
{
    if (object == nullptr)
        return;

    if (object->transform.scale.x < 0.05f)
        object->transform.scale.x = 0.05f;

    if (object->transform.scale.y < 0.05f)
        object->transform.scale.y = 0.05f;

    if (object->transform.scale.z < 0.05f)
        object->transform.scale.z = 0.05f;
}

static void DrawPrimitiveShapeControls(
    SceneObject* selectedObject
)
{
    if (!IsPrimitiveShapeObject(selectedObject))
        return;
    ImGui::Separator();

    if (!ImGui::CollapsingHeader("Primitive Shape Controls"))
    {
        return;
    }

    ImGui::TextWrapped(
        "Special controls for procedural primitives only: Ball, Cylinder, and Cone."
    );

    ImGui::Spacing();
    glm::vec3 currentScale =
        selectedObject->transform.scale;

    if (selectedObject->editorMeshType == "Sphere")
    {
        ImGui::Text("Selected Primitive: Ball / Sphere");

        float radius =
            (
                currentScale.x +
                currentScale.y +
                currentScale.z
                ) / 3.0f;

        if (
            ImGui::DragFloat(
                "Radius##SphereRadius",
                &radius,
                0.05f,
                0.05f,
                20.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                radius,
                radius,
                radius
            );
        }

        float stretch[3] =
        {
            selectedObject->transform.scale.x,
            selectedObject->transform.scale.y,
            selectedObject->transform.scale.z
        };

        if (
            ImGui::DragFloat3(
                "Stretch X/Y/Z##SphereStretch",
                stretch,
                0.05f,
                0.05f,
                20.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                stretch[0],
                stretch[1],
                stretch[2]
            );
        }

        if (ImGui::Button("Perfect Ball"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                1.5f,
                1.5f,
                1.5f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Squash Ball"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                2.2f,
                0.7f,
                2.2f
            );
        }

        if (ImGui::Button("Tall Ball"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                1.2f,
                3.0f,
                1.2f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Flat Disc Ball"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                3.0f,
                0.25f,
                3.0f
            );
        }
    }
    else if (selectedObject->editorMeshType == "Cylinder")
    {
        ImGui::Text("Selected Primitive: Cylinder");

        float radius =
            (
                selectedObject->transform.scale.x +
                selectedObject->transform.scale.z
                ) * 0.5f;

        float height =
            selectedObject->transform.scale.y;

        if (
            ImGui::DragFloat(
                "Radius##CylinderRadius",
                &radius,
                0.05f,
                0.05f,
                20.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                radius,
                selectedObject->transform.scale.y,
                radius
            );
        }

        if (
            ImGui::DragFloat(
                "Height##CylinderHeight",
                &height,
                0.05f,
                0.05f,
                30.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                selectedObject->transform.scale.x,
                height,
                selectedObject->transform.scale.z
            );
        }

        if (ImGui::Button("Pillar"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                0.8f,
                4.0f,
                0.8f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Wide Platform"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                3.5f,
                0.35f,
                3.5f
            );
        }

        if (ImGui::Button("Barrel Shape"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                1.4f,
                1.8f,
                1.4f
            );
        }
    }
    else if (selectedObject->editorMeshType == "Cone")
    {
        ImGui::Text("Selected Primitive: Cone");

        float radius =
            (
                selectedObject->transform.scale.x +
                selectedObject->transform.scale.z
                ) * 0.5f;

        float height =
            selectedObject->transform.scale.y;

        if (
            ImGui::DragFloat(
                "Radius##ConeRadius",
                &radius,
                0.05f,
                0.05f,
                20.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                radius,
                selectedObject->transform.scale.y,
                radius
            );
        }

        if (
            ImGui::DragFloat(
                "Height##ConeHeight",
                &height,
                0.05f,
                0.05f,
                30.0f
            )
            )
        {
            SetPrimitiveShapeScale(
                selectedObject,
                selectedObject->transform.scale.x,
                height,
                selectedObject->transform.scale.z
            );
        }

        if (ImGui::Button("Spike"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                0.55f,
                4.5f,
                0.55f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Wide Cone"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                2.8f,
                1.4f,
                2.8f
            );
        }

        if (ImGui::Button("Roof Cone"))
        {
            SetPrimitiveShapeScale(
                selectedObject,
                3.0f,
                1.6f,
                3.0f
            );
        }
    }

    ClampPrimitiveShapeScale(
        selectedObject
    );
}
static void DrawAdvancedObjectShapingControls(
    SceneObject* selectedObject
)
{
    if (selectedObject == nullptr)
        return;

    if (selectedObject->name == "Player")
        return;

    if (selectedObject->name == "Procedural Terrain")
        return;

    ImGui::Separator();

    if (!ImGui::CollapsingHeader("Advanced Object Shaping"))
    {
        return;
    }

    ImGui::TextWrapped(
        "Universal shaping controls for editor objects: walls, floors, pillars, platforms, primitives, and props."
    );

    ImGui::Spacing();
    glm::vec3 position =
        selectedObject->transform.position;

    if (
        ImGui::DragFloat3(
            "Position X/Y/Z##AdvancedPosition",
            &position.x,
            0.10f,
            -500.0f,
            500.0f
        )
        )
    {
        selectedObject->transform.position =
            position;
    }

    glm::vec3 scale =
        selectedObject->transform.scale;

    if (
        ImGui::DragFloat3(
            "Size X/Y/Z##AdvancedScale",
            &scale.x,
            0.05f,
            0.05f,
            80.0f
        )
        )
    {
        if (scale.x < 0.05f)
            scale.x = 0.05f;

        if (scale.y < 0.05f)
            scale.y = 0.05f;

        if (scale.z < 0.05f)
            scale.z = 0.05f;

        selectedObject->transform.scale =
            scale;
    }

    float uniformSize =
        (
            selectedObject->transform.scale.x +
            selectedObject->transform.scale.y +
            selectedObject->transform.scale.z
            ) / 3.0f;

    if (
        ImGui::DragFloat(
            "Uniform Size##AdvancedUniformSize",
            &uniformSize,
            0.05f,
            0.05f,
            80.0f
        )
        )
    {
        selectedObject->transform.scale =
            glm::vec3(
                uniformSize
            );
    }

    glm::vec3 rotation =
        selectedObject->transform.rotation;

    if (
        ImGui::DragFloat3(
            "Rotation X/Y/Z##AdvancedRotation",
            &rotation.x,
            1.0f,
            -360.0f,
            360.0f
        )
        )
    {
        selectedObject->transform.rotation =
            rotation;
    }

    ImGui::Text("Quick Rotation");

    if (ImGui::Button("Y +45"))
    {
        selectedObject->transform.rotation.y +=
            45.0f;
    }

    ImGui::SameLine();

    if (ImGui::Button("Y -45"))
    {
        selectedObject->transform.rotation.y -=
            45.0f;
    }

    ImGui::SameLine();

    if (ImGui::Button("Y +90"))
    {
        selectedObject->transform.rotation.y +=
            90.0f;
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset Rot"))
    {
        selectedObject->transform.rotation =
            glm::vec3(
                0.0f
            );
    }

    ImGui::Text("Building Presets");

    if (ImGui::Button("Wall Shape"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                6.0f,
                2.8f,
                0.35f
            );
    }

    ImGui::SameLine();

    if (ImGui::Button("Thin Wall"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                6.0f,
                2.4f,
                0.18f
            );
    }

    if (ImGui::Button("Floor Shape"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                6.0f,
                0.15f,
                6.0f
            );
    }

    ImGui::SameLine();

    if (ImGui::Button("Platform"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                5.0f,
                0.35f,
                5.0f
            );
    }

    if (ImGui::Button("Pillar Shape"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                0.65f,
                4.5f,
                0.65f
            );
    }

    ImGui::SameLine();

    if (ImGui::Button("Beam Shape"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                5.5f,
                0.35f,
                0.35f
            );
    }

    if (ImGui::Button("Reset Scale"))
    {
        selectedObject->transform.scale =
            glm::vec3(
                1.0f
            );
    }

    ImGui::SameLine();

    if (ImGui::Button("Snap To Terrain"))
    {
        selectedObject->transform.position =
            SnapEditorPositionToTerrain(
                selectedObject->transform.position,
                0.10f
            );
    }
}
static bool FileExists(
    const std::string& path
)
{
    std::ifstream file(
        path.c_str()
    );

    return file.good();
}

static std::string ResolveAssetPath(
    const std::string& relativePath
)
{
    if (FileExists(relativePath))
        return relativePath;

    std::string path1 =
        "../" + relativePath;

    if (FileExists(path1))
        return path1;

    std::string path2 =
        "../../" + relativePath;

    if (FileExists(path2))
        return path2;

    std::string path3 =
        "../../../" + relativePath;

    if (FileExists(path3))
        return path3;

    std::string path4 =
        "../../../../" + relativePath;

    if (FileExists(path4))
        return path4;

    return relativePath;
}

static std::string GetDirectoryFromPath(
    const std::string& path
)
{
    size_t slashPosition =
        path.find_last_of(
            "/\\"
        );

    if (slashPosition == std::string::npos)
    {
        return "";
    }

    return path.substr(
        0,
        slashPosition + 1
    );
}

static Material* CreateCampfireMaterial(
    const glm::vec3& tint,
    const glm::vec3& ambient,
    const glm::vec3& diffuse
)
{
    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        tint;

    material->ambient =
        ambient;

    material->diffuse =
        diffuse;

    material->specular =
        glm::vec3(
            0.05f,
            0.04f,
            0.03f
        );

    material->shininess =
        4.0f;

    return material;
}

static SceneObject* SpawnCampfire(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    (void)cubeMesh;

    if (shader == nullptr)
        return nullptr;

    static Model* campfireModel =
        nullptr;

    if (campfireModel == nullptr)
    {
        campfireModel =
            new Model(
                "Assets/Models/Environment/Campfire/campfire.obj",
                "Assets/Models/Environment/Campfire/"
            );
    }

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 center =
        camera.Position +
        forward * 7.0f;

    center =
        SnapEditorPositionToTerrain(
            center,
            0.08f
        );

    SceneObject* campfireObject =
        new SceneObject(
            campfireModel,
            shader
        );

    campfireObject->name =
        "Campfire";

    campfireObject->transform.position =
        center;

    campfireObject->transform.rotation =
        glm::vec3(
            0.0f,
            0.0f,
            0.0f
        );

    campfireObject->transform.scale =
        glm::vec3(
            1.0f
        );

    campfireObject->isCollider =
        true;

    campfireObject->colliderRadius =
        1.6f;

    campfireObject->boundingRadius =
        25.0f;
    SetEditorSaveMetadata(
        campfireObject,
        "Model",
        "None",
        "Assets/Models/Environment/Campfire/campfire.obj",
        "Assets/Models/Environment/Campfire/"
    );
    campfireObject->assetId =
        "Campfire";

    campfireObject->assetType =
        AssetType::Prop;

    campfireObject->spawnSource =
        SpawnSource::Manual;

    campfireObject->persistent =
        true;

    campfireObject->showInHierarchy =
        true;

    Light* campfireLight =
        new Light();

    campfireLight->name =
        "Campfire Light";

    campfireLight->type =
        LightType::Point;

    campfireLight->position =
        campfireObject->transform.position +
        glm::vec3(
            0.0f,
            1.25f,
            0.0f
        );

    campfireLight->color =
        glm::vec3(
            5.0f,
            2.6f,
            0.8f
        );

    scene.AddLight(
        campfireLight
    );

    campfireObject->attachedLight =
        campfireLight;

    campfireObject->attachedLightOffset =
        glm::vec3(
            0.0f,
            1.25f,
            0.0f
        );

    scene.AddObject(
        campfireObject
    );

    selectedObject =
        campfireObject;

    std::cout
        << "Campfire model spawned."
        << std::endl;

    return campfireObject;
}
void DrawHierarchyNode(
    SceneObject* obj,
    SceneObject*& selectedObject
)
{
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow;

    if (obj == selectedObject)
        flags |= ImGuiTreeNodeFlags_Selected;

    std::string id =
        obj->name +
        "##" +
        std::to_string((size_t)obj);

    bool opened =
        ImGui::TreeNodeEx(
            id.c_str(),
            flags
        );

    if (
        ImGui::IsItemClicked() &&
        !ImGui::IsItemToggledOpen()
        )
    {
        selectedObject = obj;
    }

    // ===============
    // DRAG SOURCE
    // =======

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload(
            "SCENE_OBJECT",
            &obj,
            sizeof(SceneObject*)
        );

        ImGui::Text(
            "%s",
            obj->name.c_str()
        );

        ImGui::EndDragDropSource();
    }

    // ======================
    // DRAG TARGET
    // ====
    if (ImGui::BeginDragDropTarget())
    {
        if (
            const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(
                "SCENE_OBJECT"
            )
            )
        {
            SceneObject* dragged =
                *(SceneObject**)payload->Data;

            if (
                dragged != obj &&
                dragged->parent != obj
                )
            {
                if (dragged->parent)
                {
                    auto& siblings =
                        dragged->parent->children;

                    siblings.erase(
                        std::remove(
                            siblings.begin(),
                            siblings.end(),
                            dragged
                        ),
                        siblings.end()
                    );
                }

                dragged->parent = obj;

                obj->children.push_back(
                    dragged
                );
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (opened)
    {
        for (
            SceneObject* child :
            obj->children
            )
        {
            DrawHierarchyNode(
                child,
                selectedObject
            );
        }

        ImGui::TreePop();
    }
}
// ================= HIERARCHY OBJECT GROUPING V1 =================

enum class EditorHierarchyGroup
{
    Player,
    Gameplay,
    Environment,
    Generated,
    Other
};

static bool HierarchyNameContains(
    SceneObject* object,
    const std::string& token
)
{
    if (object == nullptr)
        return false;

    return
        object->name.find(token) != std::string::npos;
}

static bool IsHierarchyPlayerObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->name == "Player")
        return true;

    if (object->assetType == AssetType::Player)
        return true;

    return false;
}

static bool IsHierarchyGeneratedObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->spawnSource == SpawnSource::Procedural)
        return true;

    if (HierarchyNameContains(object, "Generated"))
        return true;

    if (HierarchyNameContains(object, "Painted"))
        return true;

    if (HierarchyNameContains(object, "World Painter"))
        return true;

    return false;
}

static bool IsHierarchyGameplayObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (
        !object->editorGameplayType.empty() &&
        object->editorGameplayType != "None"
        )
    {
        return true;
    }

    if (object->assetType == AssetType::Gameplay)
        return true;

    if (HierarchyNameContains(object, "Coin"))
        return true;

    if (HierarchyNameContains(object, "Trigger"))
        return true;

    if (HierarchyNameContains(object, "Monster"))
        return true;

    if (HierarchyNameContains(object, "Music"))
        return true;

    if (HierarchyNameContains(object, "NPC"))
        return true;

    return false;
}

static bool IsHierarchyEnvironmentObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->assetType == AssetType::Terrain)
        return true;

    if (object->assetType == AssetType::Tree)
        return true;

    if (object->assetType == AssetType::Rock)
        return true;

    if (object->assetType == AssetType::Grass)
        return true;

    if (object->assetType == AssetType::Flower)
        return true;

    if (object->assetType == AssetType::Bush)
        return true;

    if (object->assetType == AssetType::House)
        return true;

    if (object->assetType == AssetType::Mountain)
        return true;

    if (object->assetType == AssetType::Fence)
        return true;

    if (HierarchyNameContains(object, "Tree"))
        return true;

    if (HierarchyNameContains(object, "Rock"))
        return true;

    if (HierarchyNameContains(object, "Bush"))
        return true;

    if (HierarchyNameContains(object, "Grass"))
        return true;

    if (HierarchyNameContains(object, "House"))
        return true;

    if (HierarchyNameContains(object, "Wall"))
        return true;

    return false;
}

static EditorHierarchyGroup GetEditorHierarchyGroup(
    SceneObject* object
)
{
    if (IsHierarchyPlayerObject(object))
        return EditorHierarchyGroup::Player;

    if (IsHierarchyGeneratedObject(object))
        return EditorHierarchyGroup::Generated;

    if (IsHierarchyGameplayObject(object))
        return EditorHierarchyGroup::Gameplay;

    if (IsHierarchyEnvironmentObject(object))
        return EditorHierarchyGroup::Environment;

    return EditorHierarchyGroup::Other;
}

static bool ShouldDrawObjectInHierarchyFolder(
    SceneObject* object,
    bool showGeneratedObjects,
    const char* searchBuffer
)
{
    if (object == nullptr)
        return false;

    if (!object->showInHierarchy && !showGeneratedObjects)
        return false;

    if (object->parent != nullptr)
        return false;

    if (
        searchBuffer != nullptr &&
        strlen(searchBuffer) > 0
        )
    {
        std::string objectName =
            object->name;

        if (
            objectName.find(searchBuffer) ==
            std::string::npos
            )
        {
            return false;
        }
    }

    return true;
}

static void DrawHierarchyFolder(
    Scene& scene,
    SceneObject*& selectedObject,
    EditorHierarchyGroup group,
    const char* folderName,
    bool showGeneratedObjects,
    const char* searchBuffer,
    bool openByDefault
)
{
    int count =
        0;

    for (SceneObject* object : scene.objects)
    {
        if (
            !ShouldDrawObjectInHierarchyFolder(
                object,
                showGeneratedObjects,
                searchBuffer
            )
            )
        {
            continue;
        }

        if (GetEditorHierarchyGroup(object) == group)
        {
            count++;
        }
    }

    if (count == 0)
        return;

    std::string title =
        std::string(folderName) +
        " (" +
        std::to_string(count) +
        ")";

    ImGuiTreeNodeFlags flags =
        openByDefault
        ? ImGuiTreeNodeFlags_DefaultOpen
        : 0;

    if (
        ImGui::CollapsingHeader(
            title.c_str(),
            flags
        )
        )
    {
        for (SceneObject* object : scene.objects)
        {
            if (
                !ShouldDrawObjectInHierarchyFolder(
                    object,
                    showGeneratedObjects,
                    searchBuffer
                )
                )
            {
                continue;
            }

            if (GetEditorHierarchyGroup(object) != group)
                continue;

            DrawHierarchyNode(
                object,
                selectedObject
            );
        }
    }
}
// ================= PREFAB TOOLS V2 =================

static int editorSelectedPrefabIndex =
-1;

static bool IsValidEditorPrefabIndex(
    int index
)
{
    return
        index >= 0 &&
        index < static_cast<int>(
            PrefabManager::prefabs.size()
            );
}

static Prefab* GetSelectedEditorPrefab()
{
    if (
        !IsValidEditorPrefabIndex(
            editorSelectedPrefabIndex
        )
        )
    {
        return nullptr;
    }

    return
        &PrefabManager::prefabs[
            editorSelectedPrefabIndex
        ];
}

static void ApplyPrefabTransformToSelectedObject(
    SceneObject* selectedObject
)
{
    if (selectedObject == nullptr)
    {
        std::cout
            << "Apply prefab failed: no object selected."
            << std::endl;

        return;
    }

    Prefab* prefab =
        GetSelectedEditorPrefab();

    if (prefab == nullptr)
    {
        std::cout
            << "Apply prefab failed: no prefab selected."
            << std::endl;

        return;
    }

    selectedObject->transform.position =
        prefab->position;

    selectedObject->transform.rotation =
        prefab->rotation;

    selectedObject->transform.scale =
        prefab->scale;

    std::cout
        << "Applied prefab transform to: "
        << selectedObject->name
        << std::endl;
}

static void DeleteSelectedEditorPrefab()
{
    if (
        !IsValidEditorPrefabIndex(
            editorSelectedPrefabIndex
        )
        )
    {
        return;
    }

    std::cout
        << "Deleted prefab: "
        << PrefabManager::prefabs[
            editorSelectedPrefabIndex
        ].name
        << std::endl;

            PrefabManager::prefabs.erase(
                PrefabManager::prefabs.begin() +
                editorSelectedPrefabIndex
            );

            if (
                editorSelectedPrefabIndex >=
                static_cast<int>(
                    PrefabManager::prefabs.size()
                    )
                )
            {
                editorSelectedPrefabIndex =
                    static_cast<int>(
                        PrefabManager::prefabs.size()
                        ) - 1;
            }
}
static SceneObject* SpawnSelectedEditorPrefab(
    Scene& scene,
    SceneObject*& selectedObject,
    Shader* shader,
    Camera& camera
)
{
    Prefab* prefab =
        GetSelectedEditorPrefab();

    if (prefab == nullptr)
    {
        std::cout
            << "Prefab spawn failed: no prefab selected."
            << std::endl;

        return nullptr;
    }

    if (shader == nullptr)
    {
        std::cout
            << "Prefab spawn failed: shader is null."
            << std::endl;

        return nullptr;
    }

    SceneObject* object =
        nullptr;

    // ================= MODEL PREFAB =================

    if (
        prefab->meshType == "Model" &&
        !prefab->modelPath.empty()
        )
    {
        Model* prefabModel =
            new Model(
                prefab->modelPath,
                prefab->modelDirectory
            );

        object =
            new SceneObject(
                prefabModel,
                shader
            );
    }

    // ================= UNSUPPORTED FOR NOW =================

    if (object == nullptr)
    {
        std::cout
            << "Prefab spawn failed: unsupported prefab mesh type: "
            << prefab->meshType
            << std::endl;

        return nullptr;
    }

    object->name =
        prefab->name + " Instance";

    // ================= SPAWN IN FRONT OF CAMERA =================

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 7.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.20f
        );

    object->transform.position =
        spawnPosition;

    object->transform.rotation =
        prefab->rotation;

    object->transform.scale =
        prefab->scale;

    // ================= PREFAB METADATA =================

    SetEditorSaveMetadata(
        object,
        prefab->meshType,
        prefab->gameplayType,
        prefab->modelPath,
        prefab->modelDirectory
    );

    object->editorTexturePath =
        prefab->texturePath;

    object->assetId =
        prefab->name;

    object->assetType =
        AssetType::Prop;

    object->spawnSource =
        SpawnSource::Manual;

    object->persistent =
        true;

    object->showInHierarchy =
        true;

    object->isCollider =
        prefab->isCollider;

    object->colliderRadius =
        prefab->colliderRadius;

    object->boundingRadius =
        50.0f;

    scene.AddObject(
        object
    );

    if (selectedObject != nullptr)
    {
        selectedObject->isSelected =
            false;
    }

    selectedObject =
        object;

    selectedObject->isSelected =
        true;

    std::cout
        << "Prefab spawned: "
        << prefab->name
        << std::endl;

    return object;
}
void EditorUI::DrawHierarchy(
    Scene& scene,
    SceneObject*& selectedObject,
    Light*& selectedLight,
    Shader* shader,
    Camera& camera
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            leftX,
            topY
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            leftWidth,
            hierarchyHeight
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Hierarchy");
    int visibleHierarchyCount =
        0;

    int hiddenGeneratedCount =
        0;

    int treeCount =
        0;

    int rockCount =
        0;

    int grassCount =
        0;

    int flowerCount =
        0;

    int propCount =
        0;

    for (SceneObject* obj : scene.objects)
    {
        if (obj == nullptr)
            continue;

        if (obj->assetType == AssetType::Tree)
            treeCount++;

        if (obj->assetType == AssetType::Rock)
            rockCount++;

        if (obj->assetType == AssetType::Grass)
            grassCount++;

        if (obj->assetType == AssetType::Flower)
            flowerCount++;

        if (obj->assetType == AssetType::Prop)
            propCount++;

        if (!obj->showInHierarchy)
            hiddenGeneratedCount++;
        else
            visibleHierarchyCount++;
    }

    ImGui::Text(
        "Visible: %d | Hidden: %d",
        visibleHierarchyCount,
        hiddenGeneratedCount
    );

    ImGui::Text(
        "Trees: %d | Rocks: %d | Grass: %d | Flowers: %d",
        treeCount,
        rockCount,
        grassCount,
        flowerCount
    );

    ImGui::Text(
        "Props: %d",
        propCount
    );

    ImGui::Separator();
    static char searchBuffer[128] = "";
    static bool showGeneratedObjects =
        false;

    ImGui::Checkbox(
        "Show generated objects",
        &showGeneratedObjects
    );

    ImGui::Separator();
    ImGui::InputText(
        "Search",
        searchBuffer,
        IM_ARRAYSIZE(searchBuffer)
    );

    ImGui::Separator();
    // ================= VIRTUAL HIERARCHY FOLDERS =================

    DrawHierarchyFolder(
        scene,
        selectedObject,
        EditorHierarchyGroup::Player,
        "Player",
        showGeneratedObjects,
        searchBuffer,
        true
    );

    DrawHierarchyFolder(
        scene,
        selectedObject,
        EditorHierarchyGroup::Gameplay,
        "Gameplay",
        showGeneratedObjects,
        searchBuffer,
        true
    );

    DrawHierarchyFolder(
        scene,
        selectedObject,
        EditorHierarchyGroup::Environment,
        "Environment",
        showGeneratedObjects,
        searchBuffer,
        true
    );

    DrawHierarchyFolder(
        scene,
        selectedObject,
        EditorHierarchyGroup::Generated,
        "Generated Objects",
        showGeneratedObjects,
        searchBuffer,
        false
    );

    DrawHierarchyFolder(
        scene,
        selectedObject,
        EditorHierarchyGroup::Other,
        "Other",
        showGeneratedObjects,
        searchBuffer,
        false
    );

    ImGui::Separator();

    ImGui::Text("Lights");

    for (Light* light : scene.lights)
    {

        std::string id =
            light->name +
            "##" +
            std::to_string((size_t)light);
       
        bool selected =
            (selectedLight == light);

        if (ImGui::Selectable(id.c_str(), selected))
        {
            selectedLight = light;
            selectedObject = nullptr;
        }
    }
    for (Light* light : scene.lights)
    {
        bool selected =
            (light == selectedLight);

        if (ImGui::Selectable(
            light->name.c_str(),
            selected))
        {
            selectedLight = light;
        }
    }
    ImGui::Separator();

    ImGui::Text("Prefabs");

    if (PrefabManager::prefabs.empty())
    {
        ImGui::TextDisabled(
            "No saved prefabs yet."
        );
    }
    else
    {
        ImGui::Text(
            "Saved Prefabs: %d",
            static_cast<int>(
                PrefabManager::prefabs.size()
                )
        );

        if (
            ImGui::CollapsingHeader(
                "Saved Prefabs",
                ImGuiTreeNodeFlags_DefaultOpen
            )
            )
        {
            for (
                int i = 0;
                i < static_cast<int>(
                    PrefabManager::prefabs.size()
                    );
                    i++
                )
            {
                Prefab& prefab =
                    PrefabManager::prefabs[i];

                bool selected =
                    editorSelectedPrefabIndex == i;

                std::string label =
                    prefab.name +
                    "##Prefab_" +
                    std::to_string(i);

                if (
                    ImGui::Selectable(
                        label.c_str(),
                        selected
                    )
                    )
                {
                    editorSelectedPrefabIndex =
                        i;
                }
            }
        }

        Prefab* selectedPrefab =
            GetSelectedEditorPrefab();

        if (selectedPrefab != nullptr)
        {
            ImGui::Separator();

            ImGui::Text(
                "Selected Prefab:"
            );

            ImGui::TextWrapped(
                "%s",
                selectedPrefab->name.c_str()
            );

            ImGui::Text(
                "Position: %.2f %.2f %.2f",
                selectedPrefab->position.x,
                selectedPrefab->position.y,
                selectedPrefab->position.z
            );

            ImGui::Text(
                "Rotation: %.2f %.2f %.2f",
                selectedPrefab->rotation.x,
                selectedPrefab->rotation.y,
                selectedPrefab->rotation.z
            );

            ImGui::Text(
                "Scale: %.2f %.2f %.2f",
                selectedPrefab->scale.x,
                selectedPrefab->scale.y,
                selectedPrefab->scale.z
            );
            ImGui::Separator();

            ImGui::Text(
                "Mesh Type: %s",
                selectedPrefab->meshType.c_str()
            );

            ImGui::Text(
                "Gameplay Type: %s",
                selectedPrefab->gameplayType.empty()
                ? "None"
                : selectedPrefab->gameplayType.c_str()
            );

            if (!selectedPrefab->modelPath.empty())
            {
                ImGui::TextWrapped(
                    "Model Path: %s",
                    selectedPrefab->modelPath.c_str()
                );
            }

            if (!selectedPrefab->texturePath.empty())
            {
                ImGui::TextWrapped(
                    "Texture Path: %s",
                    selectedPrefab->texturePath.c_str()
                );
            }

            ImGui::Text(
                "Collider: %s",
                selectedPrefab->isCollider
                ? "Yes"
                : "No"
            );

            ImGui::Text(
                "Collider Radius: %.2f",
                selectedPrefab->colliderRadius
            );
            if (ImGui::Button("Spawn Prefab"))
            {
                SpawnSelectedEditorPrefab(
                    scene,
                    selectedObject,
                    shader,
                    camera
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Apply To Selected"))
            {
                ApplyPrefabTransformToSelectedObject(
                    selectedObject
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete Prefab"))
            {
                DeleteSelectedEditorPrefab();
            }
            ImGui::TextDisabled(
                "V4 can spawn saved model prefabs into the scene."
            );
           
        }
    }
    ImGui::End();
}
static void ApplyMaterialPreset(
    SceneObject* object,
    const glm::vec3& tint,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float shininess
)
{
    if (object == nullptr)
        return;

    if (object->material == nullptr)
    {
        object->material =
            new Material(
                nullptr
            );
    }

    object->material->tint =
        tint;

    object->material->ambient =
        ambient;

    object->material->diffuse =
        diffuse;

    object->material->specular =
        specular;

    object->material->shininess =
        shininess;
}
static Texture* GetEditorTexture(
    const std::string& path
)
{
    static std::unordered_map<std::string, Texture*> loadedTextures;

    auto it =
        loadedTextures.find(
            path
        );

    if (it != loadedTextures.end())
    {
        return it->second;
    }

    Texture* texture =
        new Texture(
            path.c_str()
        );

    loadedTextures[path] =
        texture;

    return texture;
}

static void ApplyTexturePreset(
    SceneObject* object,
    const std::string& texturePath,
    const glm::vec3& tint,
    const glm::vec3& ambient,
    const glm::vec3& diffuse,
    const glm::vec3& specular,
    float shininess
)
{
    if (object == nullptr)
        return;

    if (object->material == nullptr)
    {
        object->material =
            new Material(
                nullptr
            );
    }

    object->material->texture =
        GetEditorTexture(
            texturePath
        );

    object->editorTexturePath =
        texturePath;
    object->material->tint =
        tint;

    object->material->ambient =
        ambient;

    object->material->diffuse =
        diffuse;

    object->material->specular =
        specular;

    object->material->shininess =
        shininess;
}

static void RemoveTextureFromObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return;

    if (object->material == nullptr)
        return;

    object->material->texture =
        nullptr;
    object->editorTexturePath =
        "";
}
struct EditorTexturePreset
{
    const char* label;
    const char* path;
    glm::vec3 tint;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

static void DrawEditorTexturePresetButton(
    SceneObject* selectedObject,
    const EditorTexturePreset& preset
)
{
    if (ImGui::Button(preset.label))
    {
        ApplyTexturePreset(
            selectedObject,
            preset.path,
            preset.tint,
            preset.ambient,
            preset.diffuse,
            preset.specular,
            preset.shininess
        );
    }
}
static Material* CloneEditorMaterial(
    Material* sourceMaterial
)

{
    if (sourceMaterial == nullptr)
    {
        return new Material(
            nullptr
        );
    }

    Material* newMaterial =
        new Material(
            sourceMaterial->texture
        );

    newMaterial->tint =
        sourceMaterial->tint;

    newMaterial->ambient =
        sourceMaterial->ambient;

    newMaterial->diffuse =
        sourceMaterial->diffuse;

    newMaterial->specular =
        sourceMaterial->specular;

    newMaterial->shininess =
        sourceMaterial->shininess;

    newMaterial->wireframe =
        sourceMaterial->wireframe;

    return newMaterial;
}

static SceneObject* DuplicateSelectedEditorObject(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    if (selectedObject == nullptr)
        return nullptr;

    if (selectedObject->editorMeshType == "Model")
    {
        std::cout
            << "Duplicate skipped: model object duplication will be added in V2."
            << std::endl;

        return nullptr;
    }

    if (selectedObject->mesh == nullptr)
        return nullptr;

    if (selectedObject->shader == nullptr)
        return nullptr;

    Material* duplicatedMaterial =
        CloneEditorMaterial(
            selectedObject->material
        );

    SceneObject* duplicatedObject =
        new SceneObject(
            selectedObject->mesh,
            selectedObject->shader,
            duplicatedMaterial
        );

    static int duplicateCounter =
        1;

    duplicatedObject->name =
        selectedObject->name +
        " Copy " +
        std::to_string(
            duplicateCounter++
        );

    duplicatedObject->transform.position =
        selectedObject->transform.position +
        glm::vec3(
            1.5f,
            0.0f,
            1.5f
        );

    duplicatedObject->transform.rotation =
        selectedObject->transform.rotation;

    duplicatedObject->transform.scale =
        selectedObject->transform.scale;

    duplicatedObject->visible =
        selectedObject->visible;

    duplicatedObject->isCollider =
        selectedObject->isCollider;

    duplicatedObject->colliderRadius =
        selectedObject->colliderRadius;

    duplicatedObject->boundingRadius =
        selectedObject->boundingRadius;

    duplicatedObject->assetId =
        selectedObject->assetId;

    duplicatedObject->assetType =
        selectedObject->assetType;

    duplicatedObject->spawnSource =
        SpawnSource::Manual;

    duplicatedObject->persistent =
        true;

    duplicatedObject->showInHierarchy =
        true;

    duplicatedObject->editorMeshType =
        selectedObject->editorMeshType;

    duplicatedObject->editorModelPath =
        selectedObject->editorModelPath;

    duplicatedObject->editorModelDirectory =
        selectedObject->editorModelDirectory;

    duplicatedObject->editorTexturePath =
        selectedObject->editorTexturePath;

    duplicatedObject->editorGameplayType =
        selectedObject->editorGameplayType;

    scene.AddObject(
        duplicatedObject
    );

    selectedObject =
        duplicatedObject;

    std::cout
        << "Duplicated object: "
        << duplicatedObject->name
        << std::endl;

    return duplicatedObject;
}
static void DrawPrimitiveMeshDetailControls(
    SceneObject* selectedObject
);
// ================= STAIRS DETAIL CONTROLS V3A =================

struct StairsDetailState
{
    int steps =
        5;
};

static std::unordered_map<SceneObject*, StairsDetailState> stairsDetailStates;

static int ClampEditorStairsSteps(
    int value
)
{
    if (value < 2)
        return 2;

    if (value > 12)
        return 12;

    return value;
}

static void AddEditorStairsVertex(
    std::vector<float>& data,
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec2& texCoord
)
{
    data.push_back(position.x);
    data.push_back(position.y);
    data.push_back(position.z);

    data.push_back(normal.x);
    data.push_back(normal.y);
    data.push_back(normal.z);

    data.push_back(texCoord.x);
    data.push_back(texCoord.y);
}

static void AddEditorStairsQuad(
    std::vector<float>& data,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& d,
    const glm::vec3& normal
)
{
    AddEditorStairsVertex(
        data,
        a,
        normal,
        glm::vec2(
            0.0f,
            0.0f
        )
    );

    AddEditorStairsVertex(
        data,
        b,
        normal,
        glm::vec2(
            1.0f,
            0.0f
        )
    );

    AddEditorStairsVertex(
        data,
        c,
        normal,
        glm::vec2(
            1.0f,
            1.0f
        )
    );

    AddEditorStairsVertex(
        data,
        a,
        normal,
        glm::vec2(
            0.0f,
            0.0f
        )
    );

    AddEditorStairsVertex(
        data,
        c,
        normal,
        glm::vec2(
            1.0f,
            1.0f
        )
    );

    AddEditorStairsVertex(
        data,
        d,
        normal,
        glm::vec2(
            0.0f,
            1.0f
        )
    );
}

static void AddEditorStairsBox(
    std::vector<float>& data,
    const glm::vec3& minPoint,
    const glm::vec3& maxPoint
)
{
    glm::vec3 p000(
        minPoint.x,
        minPoint.y,
        minPoint.z
    );

    glm::vec3 p001(
        minPoint.x,
        minPoint.y,
        maxPoint.z
    );

    glm::vec3 p010(
        minPoint.x,
        maxPoint.y,
        minPoint.z
    );

    glm::vec3 p011(
        minPoint.x,
        maxPoint.y,
        maxPoint.z
    );

    glm::vec3 p100(
        maxPoint.x,
        minPoint.y,
        minPoint.z
    );

    glm::vec3 p101(
        maxPoint.x,
        minPoint.y,
        maxPoint.z
    );

    glm::vec3 p110(
        maxPoint.x,
        maxPoint.y,
        minPoint.z
    );

    glm::vec3 p111(
        maxPoint.x,
        maxPoint.y,
        maxPoint.z
    );

    // Front
    AddEditorStairsQuad(
        data,
        p001,
        p101,
        p111,
        p011,
        glm::vec3(
            0.0f,
            0.0f,
            1.0f
        )
    );

    // Back
    AddEditorStairsQuad(
        data,
        p100,
        p000,
        p010,
        p110,
        glm::vec3(
            0.0f,
            0.0f,
            -1.0f
        )
    );

    // Left
    AddEditorStairsQuad(
        data,
        p000,
        p001,
        p011,
        p010,
        glm::vec3(
            -1.0f,
            0.0f,
            0.0f
        )
    );

    // Right
    AddEditorStairsQuad(
        data,
        p101,
        p100,
        p110,
        p111,
        glm::vec3(
            1.0f,
            0.0f,
            0.0f
        )
    );

    // Top
    AddEditorStairsQuad(
        data,
        p010,
        p011,
        p111,
        p110,
        glm::vec3(
            0.0f,
            1.0f,
            0.0f
        )
    );

    // Bottom
    AddEditorStairsQuad(
        data,
        p000,
        p100,
        p101,
        p001,
        glm::vec3(
            0.0f,
            -1.0f,
            0.0f
        )
    );
}

static Mesh* CreateEditorStairsMesh(
    int steps
)
{
    steps =
        ClampEditorStairsSteps(
            steps
        );

    std::vector<float> data;

    float stepDepth =
        1.0f / static_cast<float>(
            steps
            );

    for (int i = 0; i < steps; i++)
    {
        float zMin =
            -0.5f +
            stepDepth *
            static_cast<float>(
                i
                );

        float zMax =
            -0.5f +
            stepDepth *
            static_cast<float>(
                i + 1
                );

        float height =
            -0.5f +
            static_cast<float>(
                i + 1
                ) /
            static_cast<float>(
                steps
                );

        AddEditorStairsBox(
            data,
            glm::vec3(
                -0.5f,
                -0.5f,
                zMin
            ),
            glm::vec3(
                0.5f,
                height,
                zMax
            )
        );
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() *
            sizeof(float)
            )
    );
}

static Mesh* GetEditorStairsMesh(
    int steps
)
{
    steps =
        ClampEditorStairsSteps(
            steps
        );

    static std::unordered_map<int, Mesh*> cachedStairsMeshes;

    auto it =
        cachedStairsMeshes.find(
            steps
        );

    if (it != cachedStairsMeshes.end())
        return it->second;

    Mesh* mesh =
        CreateEditorStairsMesh(
            steps
        );

    cachedStairsMeshes[steps] =
        mesh;

    return mesh;
}

static StairsDetailState LoadStairsDetailStateFromObject(
    SceneObject* object
)
{
    StairsDetailState state;

    if (object == nullptr)
        return state;

    std::string detail =
        object->editorPrimitiveDetail;

    std::string prefix =
        "stairs_steps=";

    if (
        detail.find(
            prefix
        ) == 0
        )
    {
        std::string valueText =
            detail.substr(
                prefix.size()
            );

        state.steps =
            std::atoi(
                valueText.c_str()
            );
    }

    state.steps =
        ClampEditorStairsSteps(
            state.steps
        );

    return state;
}

static void SaveStairsDetailStateToObject(
    SceneObject* object,
    const StairsDetailState& state
)
{
    if (object == nullptr)
        return;

    int steps =
        ClampEditorStairsSteps(
            state.steps
        );

    object->editorMeshType =
        "Stairs";

    object->editorPrimitiveDetail =
        "stairs_steps=" +
        std::to_string(
            steps
        );
}

static void ApplyStairsDetailToObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return;

    StairsDetailState& state =
        stairsDetailStates[object];

    state.steps =
        ClampEditorStairsSteps(
            state.steps
        );

    object->mesh =
        GetEditorStairsMesh(
            state.steps
        );

    object->editorMeshType =
        "Stairs";

    SaveStairsDetailStateToObject(
        object,
        state
    );

    object->boundingRadius =
        2.0f;
}

static void DrawStairsDetailControls(
    SceneObject* selectedObject
)
{
    if (selectedObject == nullptr)
        return;

    if (selectedObject->editorMeshType != "Stairs")
        return;

    if (
        stairsDetailStates.find(
            selectedObject
        ) == stairsDetailStates.end()
        )
    {
        stairsDetailStates[selectedObject] =
            LoadStairsDetailStateFromObject(
                selectedObject
            );
    }

    StairsDetailState& state =
        stairsDetailStates[selectedObject];

    if (
        ImGui::CollapsingHeader(
            "Stairs Detail Controls V3A",
            ImGuiTreeNodeFlags_DefaultOpen
        )
        )
    {
        ImGui::Text(
            "Procedural stairs mesh"
        );

        ImGui::Separator();

        int steps =
            state.steps;

        if (
            ImGui::SliderInt(
                "Step Count",
                &steps,
                2,
                12
            )
            )
        {
            state.steps =
                steps;

            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        if (
            ImGui::Button(
                "Regenerate Stairs"
            )
            )
        {
            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        ImGui::Separator();

        if (
            ImGui::Button(
                "Low Stairs"
            )
            )
        {
            state.steps =
                4;

            selectedObject->transform.scale =
                glm::vec3(
                    2.5f,
                    0.45f,
                    3.0f
                );

            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        ImGui::SameLine();

        if (
            ImGui::Button(
                "Tall Stairs"
            )
            )
        {
            state.steps =
                8;

            selectedObject->transform.scale =
                glm::vec3(
                    2.0f,
                    1.8f,
                    3.0f
                );

            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        if (
            ImGui::Button(
                "Wide Stairs"
            )
            )
        {
            state.steps =
                6;

            selectedObject->transform.scale =
                glm::vec3(
                    4.0f,
                    1.0f,
                    3.0f
                );

            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        ImGui::SameLine();

        if (
            ImGui::Button(
                "Reset Stairs"
            )
            )
        {
            state.steps =
                5;

            selectedObject->transform.scale =
                glm::vec3(
                    2.0f,
                    1.0f,
                    2.5f
                );

            ApplyStairsDetailToObject(
                selectedObject
            );
        }

        ImGui::Separator();

        ImGui::Text(
            "Saved Detail:"
        );

        ImGui::TextWrapped(
            "%s",
            selectedObject->editorPrimitiveDetail.c_str()
        );
    }
}
void EditorUI::DrawInspector(
    SceneObject* selectedObject
)
{
  
    ImGui::SetNextWindowPos(
        ImVec2(
            rightX,
            topY
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            rightWidth,
            inspectorHeight
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Inspector");

    if (selectedObject != nullptr)
    {
        static char nameBuffer[128] = "";

        static SceneObject* lastSelected = nullptr;

        if (lastSelected != selectedObject)
        {
            strcpy_s(
                nameBuffer,
                selectedObject->name.c_str()
            );

            lastSelected = selectedObject;
        }

        if (ImGui::InputText(
            "Name",
            nameBuffer,
            IM_ARRAYSIZE(nameBuffer)))
        {
            selectedObject->name = nameBuffer;
        }
        // ================= SMART GAMEPLAY COMPONENT PANEL =================

        ImGui::Separator();

        ImGui::Text(
            "Object Role"
        );

        ImGui::Checkbox(
            "Visible",
            &selectedObject->visible
        );

        ImGui::Checkbox(
            "Persistent",
            &selectedObject->persistent
        );

        ImGui::Checkbox(
            "Show In Hierarchy",
            &selectedObject->showInHierarchy
        );

        ImGui::Separator();

        ImGui::Text(
            "Gameplay Component"
        );

        const char* gameplayTypes[] =
        {
            "None",
            "Coin",
            "TriggerZone",
            "MonsterSpawn",
            "MusicGate",
            "MusicNPC",
            "Obstacle"
        };

        int gameplayTypeIndex =
            0;

        if (selectedObject->editorGameplayType == "Coin")
        {
            gameplayTypeIndex =
                1;
        }
        else if (selectedObject->editorGameplayType == "TriggerZone")
        {
            gameplayTypeIndex =
                2;
        }
        else if (selectedObject->editorGameplayType == "MonsterSpawn")
        {
            gameplayTypeIndex =
                3;
        }
        else if (selectedObject->editorGameplayType == "MusicGate")
        {
            gameplayTypeIndex =
                4;
        }
        else if (selectedObject->editorGameplayType == "MusicNPC")
        {
            gameplayTypeIndex =
                5;
        }
        else if (selectedObject->editorGameplayType == "Obstacle")
        {
            gameplayTypeIndex =
                6;
        }

        if (
            ImGui::Combo(
                "Gameplay Type",
                &gameplayTypeIndex,
                gameplayTypes,
                IM_ARRAYSIZE(gameplayTypes)
            )
            )
        {
            if (gameplayTypeIndex == 0)
            {
                selectedObject->editorGameplayType =
                    "";
            }
            else
            {
                selectedObject->editorGameplayType =
                    gameplayTypes[gameplayTypeIndex];
            }

            selectedObject->persistent =
                true;
        }

        ImGui::Checkbox(
            "Is Collider",
            &selectedObject->isCollider
        );

        ImGui::DragFloat(
            "Collider Radius",
            &selectedObject->colliderRadius,
            0.05f,
            0.1f,
            50.0f
        );

        ImGui::TextDisabled(
            "Used by Play Mode, Collision Debug, and demo tools."
        );

        ImGui::Separator();

        auto ApplyGameplayRole =
            [&](
                const std::string& type,
                const std::string& newName,
                bool collider,
                float radius
                )
            {
                selectedObject->editorGameplayType =
                    type;

                selectedObject->name =
                    newName;

                selectedObject->isCollider =
                    collider;

                selectedObject->colliderRadius =
                    radius;

                selectedObject->persistent =
                    true;

                selectedObject->visible =
                    true;

                selectedObject->showInHierarchy =
                    true;

                strcpy_s(
                    nameBuffer,
                    selectedObject->name.c_str()
                );
            };

        if (ImGui::Button("Make Coin"))
        {
            ApplyGameplayRole(
                "Coin",
                "Coin",
                false,
                1.25f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Make Trigger Zone"))
        {
            ApplyGameplayRole(
                "TriggerZone",
                "Trigger Zone",
                false,
                4.5f
            );
        }

        if (ImGui::Button("Make Monster Spawn"))
        {
            ApplyGameplayRole(
                "MonsterSpawn",
                "Monster Spawn",
                false,
                2.4f
            );
        }

        if (ImGui::Button("Make Music Gate"))
        {
            ApplyGameplayRole(
                "MusicGate",
                "Music Gate",
                false,
                4.5f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Make Music NPC"))
        {
            ApplyGameplayRole(
                "MusicNPC",
                "Music NPC",
                false,
                2.0f
            );
        }

        if (ImGui::Button("Make Obstacle"))
        {
            ApplyGameplayRole(
                "Obstacle",
                "Obstacle Collider",
                true,
                glm::max(
                    selectedObject->transform.scale.x,
                    selectedObject->transform.scale.z
                ) * 0.8f
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear Role"))
        {
            selectedObject->editorGameplayType =
                "";

            selectedObject->isCollider =
                false;
        }

        ImGui::Separator();

        ImGui::Separator();
        if (ImGui::CollapsingHeader("Editor Metadata"))
        {
        ImGui::Text("Editor Metadata");

        ImGui::Text(
            "Asset Type: %s",
            GetAssetTypeName(
                selectedObject->assetType
            )
        );

        ImGui::Text(
            "Spawn Source: %s",
            GetSpawnSourceName(
                selectedObject->spawnSource
            )
        );

        ImGui::Text(
            "Mesh Type: %s",
            selectedObject->editorMeshType.c_str()
        );

        ImGui::Text(
            "Gameplay Type: %s",
            selectedObject->editorGameplayType.c_str()
        );

        ImGui::Text(
            "Persistent: %s",
            selectedObject->persistent ? "Yes" : "No"
        );

        ImGui::Text(
            "Show In Hierarchy: %s",
            selectedObject->showInHierarchy ? "Yes" : "No"
        );

        if (!selectedObject->editorModelPath.empty())
        {
            ImGui::TextWrapped(
                "Model Path: %s",
                selectedObject->editorModelPath.c_str()
            );
        }

        if (!selectedObject->editorTexturePath.empty())
        {
            ImGui::TextWrapped(
                "Texture Path: %s",
                selectedObject->editorTexturePath.c_str()
            );
        }

        }
        DrawAdvancedObjectShapingControls(
            selectedObject
        );

        DrawPrimitiveShapeControls(
            selectedObject
        );
        DrawPrimitiveMeshDetailControls(
            selectedObject
        );
        DrawStairsDetailControls(
            selectedObject
        );
        ImGui::Separator();
        ImGui::Separator();
      

        ImGui::DragFloat3(
            "Position",
            glm::value_ptr(
                selectedObject->transform.position
            ),
            0.1f
        );

        ImGui::DragFloat3(
            "Rotation",
            glm::value_ptr(
                selectedObject->transform.rotation
            ),
            1.0f
        );

        ImGui::DragFloat3(
            "Scale",
            glm::value_ptr(
                selectedObject->transform.scale
            ),
            0.1f
        );
        if (selectedObject->material != nullptr)
        {
            ImGui::Separator();
            ImGui::Text("Material");

            ImGui::ColorEdit3(
                "Ambient",
                glm::value_ptr(selectedObject->material->ambient)
            );

            ImGui::ColorEdit3(
                "Diffuse",
                glm::value_ptr(selectedObject->material->diffuse)
            );

            ImGui::ColorEdit3(
                "Specular",
                glm::value_ptr(selectedObject->material->specular)
            );

            ImGui::DragFloat(
                "Shininess",
                &selectedObject->material->shininess,
                1.0f,
                1.0f,
                256.0f
            );

            ImGui::Checkbox(
                "Wireframe",
                &selectedObject->material->wireframe
            );

            ImGui::ColorEdit3(
                "Tint",
                glm::value_ptr(selectedObject->material->tint)
            );
        }
        else
        {
            ImGui::Separator();
            ImGui::Text("Imported Model");
            ImGui::Text("No editable material yet");
        }

    }
   else
   {
       ImGui::Text("No object selected");
}

if (selectedObject != nullptr)
{
    ImGui::Separator();

    ImGui::Text("Material Presets");

    if (ImGui::Button("Wood"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.45f, 0.25f, 0.10f),
            glm::vec3(0.18f, 0.10f, 0.04f),
            glm::vec3(0.55f, 0.32f, 0.14f),
            glm::vec3(0.05f, 0.04f, 0.03f),
            6.0f
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Stone"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.45f, 0.45f, 0.42f),
            glm::vec3(0.18f, 0.18f, 0.16f),
            glm::vec3(0.55f, 0.55f, 0.50f),
            glm::vec3(0.08f, 0.08f, 0.08f),
            10.0f
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Metal"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.55f, 0.55f, 0.58f),
            glm::vec3(0.15f, 0.15f, 0.16f),
            glm::vec3(0.55f, 0.55f, 0.58f),
            glm::vec3(0.75f, 0.75f, 0.80f),
            64.0f
        );
    }

    if (ImGui::Button("Dirt"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.32f, 0.20f, 0.10f),
            glm::vec3(0.14f, 0.08f, 0.04f),
            glm::vec3(0.40f, 0.25f, 0.12f),
            glm::vec3(0.03f, 0.03f, 0.02f),
            4.0f
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Grass"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.20f, 0.45f, 0.14f),
            glm::vec3(0.08f, 0.18f, 0.05f),
            glm::vec3(0.25f, 0.55f, 0.18f),
            glm::vec3(0.03f, 0.04f, 0.03f),
            5.0f
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Dark"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(0.08f, 0.08f, 0.09f),
            glm::vec3(0.03f, 0.03f, 0.035f),
            glm::vec3(0.10f, 0.10f, 0.12f),
            glm::vec3(0.02f, 0.02f, 0.025f),
            8.0f
        );
    }

    if (ImGui::Button("Fire Glow"))
    {
        ApplyMaterialPreset(
            selectedObject,
            glm::vec3(1.0f, 0.35f, 0.05f),
            glm::vec3(0.80f, 0.22f, 0.04f),
            glm::vec3(1.0f, 0.45f, 0.08f),
            glm::vec3(0.10f, 0.06f, 0.02f),
            4.0f
        );
    }
    ImGui::Separator();

    ImGui::Text("Texture Presets");

    static EditorTexturePreset texturePresets[] =
    {
        {
            "Wall 1",
            "Assets/Textures/Materials/wall1.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.35f),
            glm::vec3(0.85f),
            glm::vec3(0.08f),
            12.0f
        },
        {
            "Wall 2",
            "Assets/Textures/Materials/wall2.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.32f, 0.24f, 0.16f),
            glm::vec3(0.85f, 0.65f, 0.42f),
            glm::vec3(0.06f, 0.05f, 0.04f),
            8.0f
        },
        {
            "Wall 3",
            "Assets/Textures/Materials/wall3.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.28f),
            glm::vec3(0.75f),
            glm::vec3(0.10f),
            14.0f
        },

        {
            "Wood Floor",
            "Assets/Textures/Materials/floor_wood.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.30f, 0.22f, 0.14f),
            glm::vec3(0.78f, 0.55f, 0.32f),
            glm::vec3(0.06f),
            8.0f
        },
        {
            "Stone Floor",
            "Assets/Textures/Materials/floor_stone.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.26f),
            glm::vec3(0.68f),
            glm::vec3(0.07f),
            10.0f
        },
        {
            "Tile Floor",
            "Assets/Textures/Materials/floor_tiles.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.30f),
            glm::vec3(0.78f),
            glm::vec3(0.10f),
            16.0f
        },

        {
            "Window Wood",
            "Assets/Textures/Materials/window_wood.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.28f, 0.18f, 0.10f),
            glm::vec3(0.70f, 0.42f, 0.22f),
            glm::vec3(0.05f),
            7.0f
        },
        {
            "Dark Window",
            "Assets/Textures/Materials/window_darkwood.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.12f, 0.08f, 0.05f),
            glm::vec3(0.35f, 0.22f, 0.12f),
            glm::vec3(0.04f),
            8.0f
        },
        {
            "Glass",
            "Assets/Textures/Materials/window_glass.jpg",
            glm::vec3(0.65f, 0.85f, 1.0f),
            glm::vec3(0.12f, 0.18f, 0.22f),
            glm::vec3(0.45f, 0.65f, 0.80f),
            glm::vec3(0.90f),
            96.0f
        },

        {
            "Roof Tiles",
            "Assets/Textures/Materials/roof_tiles.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.25f, 0.10f, 0.06f),
            glm::vec3(0.70f, 0.25f, 0.14f),
            glm::vec3(0.04f),
            6.0f
        },
        {
            "Roof Wood",
            "Assets/Textures/Materials/roof_wood.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.24f, 0.16f, 0.09f),
            glm::vec3(0.65f, 0.38f, 0.18f),
            glm::vec3(0.05f),
            7.0f
        },

        {
            "Pillar Stone",
            "Assets/Textures/Materials/pillar_stone.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.28f),
            glm::vec3(0.72f),
            glm::vec3(0.08f),
            12.0f
        },
        {
            "Marble",
            "Assets/Textures/Materials/pillar_marble.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.36f),
            glm::vec3(0.88f),
            glm::vec3(0.18f),
            24.0f
        },

        {
            "Ramp Wood",
            "Assets/Textures/Materials/ramp_wood.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.28f, 0.18f, 0.10f),
            glm::vec3(0.72f, 0.45f, 0.23f),
            glm::vec3(0.05f),
            7.0f
        },
        {
            "Dirt Path",
            "Assets/Textures/Materials/path_dirt.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.18f, 0.10f, 0.05f),
            glm::vec3(0.45f, 0.27f, 0.12f),
            glm::vec3(0.03f),
            4.0f
        },
        {
            "Stone Path",
            "Assets/Textures/Materials/path_stone.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.25f),
            glm::vec3(0.62f),
            glm::vec3(0.07f),
            10.0f
        },

        {
            "Dark Metal",
            "Assets/Textures/Materials/metal_dark.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.12f),
            glm::vec3(0.35f),
            glm::vec3(0.80f),
            80.0f
        },
        {
            "Rusty Metal",
            "Assets/Textures/Materials/metal_rusty.jpg",
            glm::vec3(1.0f),
            glm::vec3(0.24f, 0.12f, 0.06f),
            glm::vec3(0.65f, 0.30f, 0.12f),
            glm::vec3(0.35f),
            32.0f
        }
    };

    for (int i = 0; i < IM_ARRAYSIZE(texturePresets); i++)
    {
        DrawEditorTexturePresetButton(
            selectedObject,
            texturePresets[i]
        );

        if ((i + 1) % 3 != 0)
        {
            ImGui::SameLine();
        }
    }

    if (ImGui::Button("Remove Texture"))
    {
        RemoveTextureFromObject(
            selectedObject
        );
    }
}
    ImGui::End();
}
void EditorUI::DrawLightInspector(
    Light* selectedLight
)
{

    ImGui::SetNextWindowPos(
        ImVec2(
            rightX,
            topY +
            inspectorHeight +
            layoutGap
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            rightWidth,
            secondaryRightHeight
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Light Inspector");

    if (selectedLight)
    {
        static char buffer[128] = "";

        static Light* lastLight = nullptr;

        if (lastLight != selectedLight)
        {
            strcpy_s(
                buffer,
                selectedLight->name.c_str()
            );

            lastLight = selectedLight;
        }

        if (ImGui::InputText(
            "Name",
            buffer,
            IM_ARRAYSIZE(buffer)
        ))
        {
            selectedLight->name = buffer;
        }

        ImGui::DragFloat3(
            "Position",
            glm::value_ptr(
                selectedLight->position
            ),
            0.1f
        );

        ImGui::ColorEdit3(
            "Color",
            glm::value_ptr(
                selectedLight->color
            )
        );
    }
    else
    {
        ImGui::Text(
            "No light selected");
    }

    ImGui::End();
}
void EditorUI::DrawDebug(
    float deltaTime,
    int totalObjects,
    int visibleObjects,
    int culledObjects,
    SceneObject* selectedObject
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            centerX + 10.0f,
            topY + 185.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            230.0f,
            145.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Debug");


    ImGui::Text(
        "FPS: %.1f",
        1.0f / deltaTime
    );

    ImGui::Separator();

    ImGui::Text(
        "Total Objects: %d",
        totalObjects
    );

    ImGui::Text(
        "Visible Objects: %d",
        visibleObjects
    );

    ImGui::Text(
        "Culled Objects: %d",
        culledObjects
    );

    if (selectedObject != nullptr)
    {
        glm::vec3 p =
            selectedObject->transform.position;

        ImGui::Separator();

        ImGui::Text(
            "Selected Pos:"
        );

        ImGui::Text(
            "%.2f %.2f %.2f",
            p.x,
            p.y,
            p.z
        );

    }

    ImGui::End();
}
static SceneObject* SpawnCampModel(
    Scene& scene,
    Model* model,
    Shader* shader,
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& rotation,
    const glm::vec3& scale,
    bool isCollider,
    float colliderRadius
)
{
    SceneObject* obj =
        new SceneObject(model, shader);

    obj->name = name;
    glm::vec3 terrainPosition =
        position;

    float localYOffset =
        position.y;

    terrainPosition.y =
        GetTerrainHeight(
            position.x,
            position.z
        ) +
        localYOffset +
        0.05f;

    obj->transform.position =
        terrainPosition;
    obj->transform.rotation = rotation;
    obj->transform.scale = scale;

    obj->isCollider = isCollider;
    obj->colliderRadius = colliderRadius;
    obj->boundingRadius = colliderRadius;

    scene.AddObject(obj);

    return obj;
}

static void BuildRealCamp(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader,
    Model* woodenHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel
)
{
    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }

    forward = glm::normalize(forward);

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(0.0f, 1.0f, 0.0f)
            )
        );

    glm::vec3 campCenter =
        camera.Position + forward * 18.0f;

    campCenter.y = 0.0f;

    // ================= HOUSE =================
    SceneObject* house =
        SpawnCampModel(
            scene,
            woodenHouseModel,
            shader,
            "Camp House",
            campCenter,
            glm::vec3(0.0f, 180.0f, 0.0f),
            glm::vec3(1.0f),
            true,
            6.0f
        );

    selectedObject = house;

    // ================= TREES =================
    SpawnCampModel(
        scene,
        pineTreeModel,
        shader,
        "Camp Tree 1",
        campCenter + right * -9.0f + forward * 6.0f,
        glm::vec3(0.0f, 25.0f, 0.0f),
        glm::vec3(2.5f),
        true,
        2.5f
    );

    SpawnCampModel(
        scene,
        pineTreeModel,
        shader,
        "Camp Tree 2",
        campCenter + right * 10.0f + forward * 7.0f,
        glm::vec3(0.0f, -10.0f, 0.0f),
        glm::vec3(2.8f),
        true,
        2.5f
    );

    SpawnCampModel(
        scene,
        commonTreeModel,
        shader,
        "Camp Tree 3",
        campCenter + right * -12.0f - forward * 3.0f,
        glm::vec3(0.0f, 40.0f, 0.0f),
        glm::vec3(2.2f),
        true,
        2.5f
    );

    SpawnCampModel(
        scene,
        commonTreeModel,
        shader,
        "Camp Tree 4",
        campCenter + right * 12.0f - forward * 5.0f,
        glm::vec3(0.0f, -35.0f, 0.0f),
        glm::vec3(2.4f),
        true,
        2.5f
    );

    // ================= ROCKS =================
    SpawnCampModel(
        scene,
        rockModel,
        shader,
        "Camp Rock 1",
        campCenter + right * -5.0f + forward * 2.0f,
        glm::vec3(0.0f, 20.0f, 0.0f),
        glm::vec3(1.8f),
        true,
        1.5f
    );

    SpawnCampModel(
        scene,
        rockModel,
        shader,
        "Camp Rock 2",
        campCenter + right * 6.0f + forward * 1.5f,
        glm::vec3(0.0f, -30.0f, 0.0f),
        glm::vec3(1.5f),
        true,
        1.5f
    );

    SpawnCampModel(
        scene,
        rockModel,
        shader,
        "Camp Rock 3",
        campCenter + right * 3.0f - forward * 6.0f,
        glm::vec3(0.0f, 10.0f, 0.0f),
        glm::vec3(1.7f),
        true,
        1.5f
    );

    // ================= BUSHES =================
    SpawnCampModel(
        scene,
        bushModel,
        shader,
        "Camp Bush 1",
        campCenter + right * -4.0f - forward * 5.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.4f),
        false,
        1.0f
    );

    SpawnCampModel(
        scene,
        bushModel,
        shader,
        "Camp Bush 2",
        campCenter + right * 5.0f - forward * 4.5f,
        glm::vec3(0.0f, 20.0f, 0.0f),
        glm::vec3(1.5f),
        false,
        1.0f
    );

    SpawnCampModel(
        scene,
        bushModel,
        shader,
        "Camp Bush 3",
        campCenter + right * 8.0f + forward * 5.0f,
        glm::vec3(0.0f, -10.0f, 0.0f),
        glm::vec3(1.3f),
        false,
        1.0f
    );

    // ================= CAMP CENTER =================
    glm::vec3 campFireArea =
        campCenter - forward * 5.0f;

    SpawnCampModel(
        scene,
        treeStumpModel,
        shader,
        "Camp Stump",
        campFireArea + right * -2.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.6f),
        false,
        1.0f
    );

    SpawnCampModel(
        scene,
        woodLogModel,
        shader,
        "Camp Log 1",
        campFireArea + right * 1.8f,
        glm::vec3(0.0f, 25.0f, 0.0f),
        glm::vec3(1.8f),
        false,
        1.0f
    );

    SpawnCampModel(
        scene,
        woodLogModel,
        shader,
        "Camp Log 2",
        campFireArea + forward * 1.5f,
        glm::vec3(0.0f, -25.0f, 0.0f),
        glm::vec3(1.8f),
        false,
        1.0f
    );
}
static SceneObject* SpawnAssetInFrontOfCamera(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader,
    Model* model,
    const std::string& objectName,
    float uniformScale,
    bool collider,
    float colliderRadius,
    float boundingRadius
)
{
    SceneObject* obj = new SceneObject(model, shader);
    obj->name = objectName;
    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    obj->transform.position =
        camera.Position +
        forward * 6.0f;

    obj->transform.position.y =
        GetObjectTerrainY(
            obj->transform.position.x,
            obj->transform.position.z,
            0.05f
        );
    obj->transform.scale = glm::vec3(uniformScale);
    obj->isCollider = collider;
    obj->colliderRadius = colliderRadius;
    obj->boundingRadius = boundingRadius;

    scene.AddObject(obj);
    selectedObject = obj;

    return obj;
}
static void AddPrimitiveVertex(
    std::vector<float>& data,
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec2& texCoord
)
{
    data.push_back(position.x);
    data.push_back(position.y);
    data.push_back(position.z);

    data.push_back(normal.x);
    data.push_back(normal.y);
    data.push_back(normal.z);

    data.push_back(texCoord.x);
    data.push_back(texCoord.y);
}

static Mesh* CreateProceduralSphereMesh(
    int rings,
    int sectors
)
{
    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float radius =
        0.5f;

    for (int r = 0; r < rings; r++)
    {
        float v0 =
            (float)r /
            (float)rings;

        float v1 =
            (float)(r + 1) /
            (float)rings;

        float phi0 =
            v0 * pi;

        float phi1 =
            v1 * pi;

        for (int s = 0; s < sectors; s++)
        {
            float u0 =
                (float)s /
                (float)sectors;

            float u1 =
                (float)(s + 1) /
                (float)sectors;

            float theta0 =
                u0 * pi * 2.0f;

            float theta1 =
                u1 * pi * 2.0f;

            glm::vec3 p00 =
                glm::vec3(
                    std::sin(phi0) * std::cos(theta0),
                    std::cos(phi0),
                    std::sin(phi0) * std::sin(theta0)
                ) * radius;

            glm::vec3 p01 =
                glm::vec3(
                    std::sin(phi0) * std::cos(theta1),
                    std::cos(phi0),
                    std::sin(phi0) * std::sin(theta1)
                ) * radius;

            glm::vec3 p10 =
                glm::vec3(
                    std::sin(phi1) * std::cos(theta0),
                    std::cos(phi1),
                    std::sin(phi1) * std::sin(theta0)
                ) * radius;

            glm::vec3 p11 =
                glm::vec3(
                    std::sin(phi1) * std::cos(theta1),
                    std::cos(phi1),
                    std::sin(phi1) * std::sin(theta1)
                ) * radius;

            glm::vec3 n00 =
                glm::normalize(
                    p00
                );

            glm::vec3 n01 =
                glm::normalize(
                    p01
                );

            glm::vec3 n10 =
                glm::normalize(
                    p10
                );

            glm::vec3 n11 =
                glm::normalize(
                    p11
                );

            AddPrimitiveVertex(
                data,
                p00,
                n00,
                glm::vec2(u0, v0)
            );

            AddPrimitiveVertex(
                data,
                p10,
                n10,
                glm::vec2(u0, v1)
            );

            AddPrimitiveVertex(
                data,
                p01,
                n01,
                glm::vec2(u1, v0)
            );

            AddPrimitiveVertex(
                data,
                p01,
                n01,
                glm::vec2(u1, v0)
            );

            AddPrimitiveVertex(
                data,
                p10,
                n10,
                glm::vec2(u0, v1)
            );

            AddPrimitiveVertex(
                data,
                p11,
                n11,
                glm::vec2(u1, v1)
            );
        }
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static Mesh* CreateProceduralCylinderMesh(
    int sectors
)
{
    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float radius =
        0.5f;

    float halfHeight =
        0.5f;

    for (int i = 0; i < sectors; i++)
    {
        float u0 =
            (float)i /
            (float)sectors;

        float u1 =
            (float)(i + 1) /
            (float)sectors;

        float a0 =
            u0 * pi * 2.0f;

        float a1 =
            u1 * pi * 2.0f;

        glm::vec3 b0 =
            glm::vec3(
                std::cos(a0) * radius,
                -halfHeight,
                std::sin(a0) * radius
            );

        glm::vec3 b1 =
            glm::vec3(
                std::cos(a1) * radius,
                -halfHeight,
                std::sin(a1) * radius
            );

        glm::vec3 t0 =
            glm::vec3(
                std::cos(a0) * radius,
                halfHeight,
                std::sin(a0) * radius
            );

        glm::vec3 t1 =
            glm::vec3(
                std::cos(a1) * radius,
                halfHeight,
                std::sin(a1) * radius
            );

        glm::vec3 n0 =
            glm::normalize(
                glm::vec3(
                    b0.x,
                    0.0f,
                    b0.z
                )
            );

        glm::vec3 n1 =
            glm::normalize(
                glm::vec3(
                    b1.x,
                    0.0f,
                    b1.z
                )
            );

        AddPrimitiveVertex(data, b0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertex(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, t0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertex(data, t0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertex(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, t1, n1, glm::vec2(u1, 1.0f));

        glm::vec3 bottomCenter =
            glm::vec3(
                0.0f,
                -halfHeight,
                0.0f
            );

        glm::vec3 topCenter =
            glm::vec3(
                0.0f,
                halfHeight,
                0.0f
            );

        AddPrimitiveVertex(data, bottomCenter, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f));
        AddPrimitiveVertex(data, b1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
        AddPrimitiveVertex(data, b0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));

        AddPrimitiveVertex(data, topCenter, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f, 0.5f));
        AddPrimitiveVertex(data, t0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, t1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static Mesh* CreateProceduralConeMesh(
    int sectors
)
{
    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float radius =
        0.5f;

    float halfHeight =
        0.5f;

    glm::vec3 top =
        glm::vec3(
            0.0f,
            halfHeight,
            0.0f
        );

    glm::vec3 bottomCenter =
        glm::vec3(
            0.0f,
            -halfHeight,
            0.0f
        );

    for (int i = 0; i < sectors; i++)
    {
        float u0 =
            (float)i /
            (float)sectors;

        float u1 =
            (float)(i + 1) /
            (float)sectors;

        float a0 =
            u0 * pi * 2.0f;

        float a1 =
            u1 * pi * 2.0f;

        glm::vec3 b0 =
            glm::vec3(
                std::cos(a0) * radius,
                -halfHeight,
                std::sin(a0) * radius
            );

        glm::vec3 b1 =
            glm::vec3(
                std::cos(a1) * radius,
                -halfHeight,
                std::sin(a1) * radius
            );

        glm::vec3 sideNormal =
            glm::normalize(
                glm::cross(
                    b1 - b0,
                    top - b0
                )
            );

        AddPrimitiveVertex(data, b0, sideNormal, glm::vec2(u0, 0.0f));
        AddPrimitiveVertex(data, b1, sideNormal, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, top, sideNormal, glm::vec2(0.5f, 1.0f));

        AddPrimitiveVertex(data, bottomCenter, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f, 0.5f));
        AddPrimitiveVertex(data, b0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, b1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static void AddBoxToPrimitiveMesh(
    std::vector<float>& data,
    const glm::vec3& minPoint,
    const glm::vec3& maxPoint
)
{
    glm::vec3 p000(minPoint.x, minPoint.y, minPoint.z);
    glm::vec3 p001(minPoint.x, minPoint.y, maxPoint.z);
    glm::vec3 p010(minPoint.x, maxPoint.y, minPoint.z);
    glm::vec3 p011(minPoint.x, maxPoint.y, maxPoint.z);

    glm::vec3 p100(maxPoint.x, minPoint.y, minPoint.z);
    glm::vec3 p101(maxPoint.x, minPoint.y, maxPoint.z);
    glm::vec3 p110(maxPoint.x, maxPoint.y, minPoint.z);
    glm::vec3 p111(maxPoint.x, maxPoint.y, maxPoint.z);

    // Front
    AddPrimitiveVertex(data, p001, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p101, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p111, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p001, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p111, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p011, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f));

    // Back
    AddPrimitiveVertex(data, p100, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p000, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p010, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p100, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p010, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p110, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f));

    // Left
    AddPrimitiveVertex(data, p000, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p001, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p011, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p000, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p011, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p010, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    // Right
    AddPrimitiveVertex(data, p101, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p100, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p110, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p101, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p110, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p111, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    // Top
    AddPrimitiveVertex(data, p010, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p011, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p111, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p010, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p111, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p110, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    // Bottom
    AddPrimitiveVertex(data, p000, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p100, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, p101, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

    AddPrimitiveVertex(data, p000, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, p101, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertex(data, p001, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
}

static Mesh* CreateProceduralStairsMesh(
    int steps
)
{
    if (steps < 2)
        steps = 2;

    if (steps > 12)
        steps = 12;

    std::vector<float> data;

    float stepDepth =
        1.0f / (float)steps;

    for (int i = 0; i < steps; i++)
    {
        float zMin =
            -0.5f + stepDepth * (float)i;

        float zMax =
            -0.5f + stepDepth * (float)(i + 1);

        float height =
            -0.5f + ((float)(i + 1) / (float)steps);

        AddBoxToPrimitiveMesh(
            data,
            glm::vec3(
                -0.5f,
                -0.5f,
                zMin
            ),
            glm::vec3(
                0.5f,
                height,
                zMax
            )
        );
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static Mesh* CreateProceduralRingMesh(
    int majorSegments,
    int minorSegments
)
{
    if (majorSegments < 6)
        majorSegments = 6;

    if (majorSegments > 96)
        majorSegments = 96;

    if (minorSegments < 4)
        minorSegments = 4;

    if (minorSegments > 48)
        minorSegments = 48;

    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float majorRadius =
        0.35f;

    float minorRadius =
        0.14f;

    for (int i = 0; i < majorSegments; i++)
    {
        float u0 =
            (float)i / (float)majorSegments * pi * 2.0f;

        float u1 =
            (float)(i + 1) / (float)majorSegments * pi * 2.0f;

        for (int j = 0; j < minorSegments; j++)
        {
            float v0 =
                (float)j / (float)minorSegments * pi * 2.0f;

            float v1 =
                (float)(j + 1) / (float)minorSegments * pi * 2.0f;

            glm::vec3 p00 =
                glm::vec3(
                    (majorRadius + minorRadius * std::cos(v0)) * std::cos(u0),
                    minorRadius * std::sin(v0),
                    (majorRadius + minorRadius * std::cos(v0)) * std::sin(u0)
                );

            glm::vec3 p01 =
                glm::vec3(
                    (majorRadius + minorRadius * std::cos(v1)) * std::cos(u0),
                    minorRadius * std::sin(v1),
                    (majorRadius + minorRadius * std::cos(v1)) * std::sin(u0)
                );

            glm::vec3 p10 =
                glm::vec3(
                    (majorRadius + minorRadius * std::cos(v0)) * std::cos(u1),
                    minorRadius * std::sin(v0),
                    (majorRadius + minorRadius * std::cos(v0)) * std::sin(u1)
                );

            glm::vec3 p11 =
                glm::vec3(
                    (majorRadius + minorRadius * std::cos(v1)) * std::cos(u1),
                    minorRadius * std::sin(v1),
                    (majorRadius + minorRadius * std::cos(v1)) * std::sin(u1)
                );

            glm::vec3 n00 =
                glm::normalize(
                    glm::vec3(
                        std::cos(v0) * std::cos(u0),
                        std::sin(v0),
                        std::cos(v0) * std::sin(u0)
                    )
                );

            glm::vec3 n01 =
                glm::normalize(
                    glm::vec3(
                        std::cos(v1) * std::cos(u0),
                        std::sin(v1),
                        std::cos(v1) * std::sin(u0)
                    )
                );

            glm::vec3 n10 =
                glm::normalize(
                    glm::vec3(
                        std::cos(v0) * std::cos(u1),
                        std::sin(v0),
                        std::cos(v0) * std::sin(u1)
                    )
                );

            glm::vec3 n11 =
                glm::normalize(
                    glm::vec3(
                        std::cos(v1) * std::cos(u1),
                        std::sin(v1),
                        std::cos(v1) * std::sin(u1)
                    )
                );

            AddPrimitiveVertex(data, p00, n00, glm::vec2(0.0f, 0.0f));
            AddPrimitiveVertex(data, p10, n10, glm::vec2(1.0f, 0.0f));
            AddPrimitiveVertex(data, p01, n01, glm::vec2(0.0f, 1.0f));

            AddPrimitiveVertex(data, p01, n01, glm::vec2(0.0f, 1.0f));
            AddPrimitiveVertex(data, p10, n10, glm::vec2(1.0f, 0.0f));
            AddPrimitiveVertex(data, p11, n11, glm::vec2(1.0f, 1.0f));
        }
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static Mesh* CreateProceduralPipeMesh(
    int sectors
)
{
    if (sectors < 6)
        sectors = 6;

    if (sectors > 96)
        sectors = 96;

    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float outerRadius =
        0.5f;

    float innerRadius =
        0.28f;

    float halfHeight =
        0.5f;

    for (int i = 0; i < sectors; i++)
    {
        float u0 =
            (float)i / (float)sectors;

        float u1 =
            (float)(i + 1) / (float)sectors;

        float a0 =
            u0 * pi * 2.0f;

        float a1 =
            u1 * pi * 2.0f;

        glm::vec3 ob0(std::cos(a0) * outerRadius, -halfHeight, std::sin(a0) * outerRadius);
        glm::vec3 ob1(std::cos(a1) * outerRadius, -halfHeight, std::sin(a1) * outerRadius);
        glm::vec3 ot0(std::cos(a0) * outerRadius, halfHeight, std::sin(a0) * outerRadius);
        glm::vec3 ot1(std::cos(a1) * outerRadius, halfHeight, std::sin(a1) * outerRadius);

        glm::vec3 ib0(std::cos(a0) * innerRadius, -halfHeight, std::sin(a0) * innerRadius);
        glm::vec3 ib1(std::cos(a1) * innerRadius, -halfHeight, std::sin(a1) * innerRadius);
        glm::vec3 it0(std::cos(a0) * innerRadius, halfHeight, std::sin(a0) * innerRadius);
        glm::vec3 it1(std::cos(a1) * innerRadius, halfHeight, std::sin(a1) * innerRadius);

        glm::vec3 n0 =
            glm::normalize(
                glm::vec3(
                    std::cos(a0),
                    0.0f,
                    std::sin(a0)
                )
            );

        glm::vec3 n1 =
            glm::normalize(
                glm::vec3(
                    std::cos(a1),
                    0.0f,
                    std::sin(a1)
                )
            );

        // Outer wall
        AddPrimitiveVertex(data, ob0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertex(data, ob1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, ot0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertex(data, ot0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertex(data, ob1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, ot1, n1, glm::vec2(u1, 1.0f));

        // Inner wall
        AddPrimitiveVertex(data, ib1, -n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, ib0, -n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertex(data, it0, -n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertex(data, ib1, -n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, it0, -n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertex(data, it1, -n1, glm::vec2(u1, 1.0f));

        // Top ring face
        AddPrimitiveVertex(data, it0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, ot0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
        AddPrimitiveVertex(data, ot1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

        AddPrimitiveVertex(data, it0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, ot1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
        AddPrimitiveVertex(data, it1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

        // Bottom ring face
        AddPrimitiveVertex(data, ib0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, ib1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
        AddPrimitiveVertex(data, ob1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

        AddPrimitiveVertex(data, ib0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertex(data, ob1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
        AddPrimitiveVertex(data, ob0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static void AddTriangleToPrimitiveMesh(
    std::vector<float>& data,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c
)
{
    glm::vec3 normal =
        glm::normalize(
            glm::cross(
                b - a,
                c - a
            )
        );

    AddPrimitiveVertex(data, a, normal, glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertex(data, b, normal, glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertex(data, c, normal, glm::vec2(0.5f, 1.0f));
}

static void AddQuadToPrimitiveMesh(
    std::vector<float>& data,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& d
)
{
    AddTriangleToPrimitiveMesh(
        data,
        a,
        b,
        c
    );

    AddTriangleToPrimitiveMesh(
        data,
        a,
        c,
        d
    );
}

static Mesh* CreateProceduralCapsuleMesh(
    int rings,
    int sectors
)
{
    if (rings < 4)
        rings = 4;

    if (rings > 32)
        rings = 32;

    if (sectors < 8)
        sectors = 8;

    if (sectors > 96)
        sectors = 96;

    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float radius =
        0.28f;

    float halfBody =
        0.25f;

    // Cylinder body
    for (int i = 0; i < sectors; i++)
    {
        float u0 =
            (float)i / (float)sectors;

        float u1 =
            (float)(i + 1) / (float)sectors;

        float a0 =
            u0 * pi * 2.0f;

        float a1 =
            u1 * pi * 2.0f;

        glm::vec3 b0(
            std::cos(a0) * radius,
            -halfBody,
            std::sin(a0) * radius
        );

        glm::vec3 b1(
            std::cos(a1) * radius,
            -halfBody,
            std::sin(a1) * radius
        );

        glm::vec3 t0(
            std::cos(a0) * radius,
            halfBody,
            std::sin(a0) * radius
        );

        glm::vec3 t1(
            std::cos(a1) * radius,
            halfBody,
            std::sin(a1) * radius
        );

        glm::vec3 n0 =
            glm::normalize(
                glm::vec3(
                    std::cos(a0),
                    0.0f,
                    std::sin(a0)
                )
            );

        glm::vec3 n1 =
            glm::normalize(
                glm::vec3(
                    std::cos(a1),
                    0.0f,
                    std::sin(a1)
                )
            );

        AddPrimitiveVertex(data, b0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertex(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, t0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertex(data, t0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertex(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertex(data, t1, n1, glm::vec2(u1, 1.0f));
    }

    // Top hemisphere
    for (int r = 0; r < rings; r++)
    {
        float v0 =
            (float)r / (float)rings;

        float v1 =
            (float)(r + 1) / (float)rings;

        float phi0 =
            v0 * pi * 0.5f;

        float phi1 =
            v1 * pi * 0.5f;

        for (int s = 0; s < sectors; s++)
        {
            float u0 =
                (float)s / (float)sectors;

            float u1 =
                (float)(s + 1) / (float)sectors;

            float theta0 =
                u0 * pi * 2.0f;

            float theta1 =
                u1 * pi * 2.0f;

            glm::vec3 n00(
                std::sin(phi0) * std::cos(theta0),
                std::cos(phi0),
                std::sin(phi0) * std::sin(theta0)
            );

            glm::vec3 n01(
                std::sin(phi0) * std::cos(theta1),
                std::cos(phi0),
                std::sin(phi0) * std::sin(theta1)
            );

            glm::vec3 n10(
                std::sin(phi1) * std::cos(theta0),
                std::cos(phi1),
                std::sin(phi1) * std::sin(theta0)
            );

            glm::vec3 n11(
                std::sin(phi1) * std::cos(theta1),
                std::cos(phi1),
                std::sin(phi1) * std::sin(theta1)
            );

            glm::vec3 p00 =
                n00 * radius +
                glm::vec3(
                    0.0f,
                    halfBody,
                    0.0f
                );

            glm::vec3 p01 =
                n01 * radius +
                glm::vec3(
                    0.0f,
                    halfBody,
                    0.0f
                );

            glm::vec3 p10 =
                n10 * radius +
                glm::vec3(
                    0.0f,
                    halfBody,
                    0.0f
                );

            glm::vec3 p11 =
                n11 * radius +
                glm::vec3(
                    0.0f,
                    halfBody,
                    0.0f
                );

            AddPrimitiveVertex(data, p00, glm::normalize(n00), glm::vec2(u0, v0));
            AddPrimitiveVertex(data, p10, glm::normalize(n10), glm::vec2(u0, v1));
            AddPrimitiveVertex(data, p01, glm::normalize(n01), glm::vec2(u1, v0));

            AddPrimitiveVertex(data, p01, glm::normalize(n01), glm::vec2(u1, v0));
            AddPrimitiveVertex(data, p10, glm::normalize(n10), glm::vec2(u0, v1));
            AddPrimitiveVertex(data, p11, glm::normalize(n11), glm::vec2(u1, v1));
        }
    }

    // Bottom hemisphere
    for (int r = 0; r < rings; r++)
    {
        float v0 =
            (float)r / (float)rings;

        float v1 =
            (float)(r + 1) / (float)rings;

        float phi0 =
            v0 * pi * 0.5f;

        float phi1 =
            v1 * pi * 0.5f;

        for (int s = 0; s < sectors; s++)
        {
            float u0 =
                (float)s / (float)sectors;

            float u1 =
                (float)(s + 1) / (float)sectors;

            float theta0 =
                u0 * pi * 2.0f;

            float theta1 =
                u1 * pi * 2.0f;

            glm::vec3 n00(
                std::sin(phi0) * std::cos(theta0),
                -std::cos(phi0),
                std::sin(phi0) * std::sin(theta0)
            );

            glm::vec3 n01(
                std::sin(phi0) * std::cos(theta1),
                -std::cos(phi0),
                std::sin(phi0) * std::sin(theta1)
            );

            glm::vec3 n10(
                std::sin(phi1) * std::cos(theta0),
                -std::cos(phi1),
                std::sin(phi1) * std::sin(theta0)
            );

            glm::vec3 n11(
                std::sin(phi1) * std::cos(theta1),
                -std::cos(phi1),
                std::sin(phi1) * std::sin(theta1)
            );

            glm::vec3 p00 =
                n00 * radius +
                glm::vec3(
                    0.0f,
                    -halfBody,
                    0.0f
                );

            glm::vec3 p01 =
                n01 * radius +
                glm::vec3(
                    0.0f,
                    -halfBody,
                    0.0f
                );

            glm::vec3 p10 =
                n10 * radius +
                glm::vec3(
                    0.0f,
                    -halfBody,
                    0.0f
                );

            glm::vec3 p11 =
                n11 * radius +
                glm::vec3(
                    0.0f,
                    -halfBody,
                    0.0f
                );

            AddPrimitiveVertex(data, p00, glm::normalize(n00), glm::vec2(u0, v0));
            AddPrimitiveVertex(data, p01, glm::normalize(n01), glm::vec2(u1, v0));
            AddPrimitiveVertex(data, p10, glm::normalize(n10), glm::vec2(u0, v1));

            AddPrimitiveVertex(data, p01, glm::normalize(n01), glm::vec2(u1, v0));
            AddPrimitiveVertex(data, p11, glm::normalize(n11), glm::vec2(u1, v1));
            AddPrimitiveVertex(data, p10, glm::normalize(n10), glm::vec2(u0, v1));
        }
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static Mesh* CreateProceduralRoofWedgeMesh()
{
    std::vector<float> data;

    glm::vec3 a(
        -0.5f,
        -0.5f,
        -0.5f
    );

    glm::vec3 b(
        0.5f,
        -0.5f,
        -0.5f
    );

    glm::vec3 c(
        0.5f,
        -0.5f,
        0.5f
    );

    glm::vec3 d(
        -0.5f,
        -0.5f,
        0.5f
    );

    glm::vec3 e(
        -0.5f,
        0.5f,
        0.0f
    );

    glm::vec3 f(
        0.5f,
        0.5f,
        0.0f
    );

    // Bottom
    AddQuadToPrimitiveMesh(
        data,
        a,
        b,
        c,
        d
    );

    // Back slope
    AddQuadToPrimitiveMesh(
        data,
        a,
        e,
        f,
        b
    );

    // Front slope
    AddQuadToPrimitiveMesh(
        data,
        d,
        c,
        f,
        e
    );

    // Left triangle
    AddTriangleToPrimitiveMesh(
        data,
        a,
        d,
        e
    );

    // Right triangle
    AddTriangleToPrimitiveMesh(
        data,
        b,
        f,
        c
    );

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static Mesh* GetProceduralPrimitiveMesh(
    const std::string& primitiveType
)
{
    static Mesh* sphereMesh =
        nullptr;

    static Mesh* cylinderMesh =
        nullptr;

    static Mesh* coneMesh =
        nullptr;

    if (primitiveType == "Sphere")
    {
        if (sphereMesh == nullptr)
        {
            sphereMesh =
                CreateProceduralSphereMesh(
                    24,
                    32
                );
        }

        return sphereMesh;
    }

    if (primitiveType == "Cylinder")
    {
        if (cylinderMesh == nullptr)
        {
            cylinderMesh =
                CreateProceduralCylinderMesh(
                    32
                );
        }

        return cylinderMesh;
    }

    if (primitiveType == "Cone")
    {
        if (coneMesh == nullptr)
        {
            coneMesh =
                CreateProceduralConeMesh(
                    32
                );
        }

        return coneMesh;
    }

    return nullptr;
}
struct PrimitiveMeshDetailState
{
    int sphereRings = 24;
    int sphereSectors = 32;
    int cylinderSides = 32;
    int coneSides = 32;
};

static std::unordered_map<SceneObject*, PrimitiveMeshDetailState>
primitiveMeshDetailStates;

static int ClampPrimitiveMeshInt(
    int value,
    int minValue,
    int maxValue
)
{
    if (value < minValue)
        return minValue;

    if (value > maxValue)
        return maxValue;

    return value;
}

static void LoadPrimitiveDetailStateFromObject(
    SceneObject* object,
    PrimitiveMeshDetailState& state
)
{
    if (object == nullptr)
        return;

    if (object->editorPrimitiveDetail.empty())
        return;

    std::stringstream stream(
        object->editorPrimitiveDetail
    );

    stream
        >> state.sphereRings
        >> state.sphereSectors
        >> state.cylinderSides
        >> state.coneSides;
}

static void SavePrimitiveDetailStateToObject(
    SceneObject* object,
    PrimitiveMeshDetailState& state
)
{
    if (object == nullptr)
        return;

    object->editorPrimitiveDetail =
        std::to_string(state.sphereRings) + " " +
        std::to_string(state.sphereSectors) + " " +
        std::to_string(state.cylinderSides) + " " +
        std::to_string(state.coneSides);
}

static PrimitiveMeshDetailState& GetPrimitiveMeshDetailState(
    SceneObject* object
)
{
    auto it =
        primitiveMeshDetailStates.find(
            object
        );

    if (it != primitiveMeshDetailStates.end())
    {
        return it->second;
    }

    PrimitiveMeshDetailState state;

    LoadPrimitiveDetailStateFromObject(
        object,
        state
    );

    auto result =
        primitiveMeshDetailStates.emplace(
            object,
            state
        );

    return result.first->second;
}

static void ApplyPrimitiveMeshDetail(
    SceneObject* selectedObject
)
{
    if (!IsPrimitiveShapeObject(selectedObject))
        return;

    PrimitiveMeshDetailState& state =
        GetPrimitiveMeshDetailState(
            selectedObject
        );

    state.sphereRings =
        ClampPrimitiveMeshInt(
            state.sphereRings,
            4,
            64
        );

    state.sphereSectors =
        ClampPrimitiveMeshInt(
            state.sphereSectors,
            6,
            96
        );

    state.cylinderSides =
        ClampPrimitiveMeshInt(
            state.cylinderSides,
            3,
            96
        );

    state.coneSides =
        ClampPrimitiveMeshInt(
            state.coneSides,
            3,
            96
        );
    SavePrimitiveDetailStateToObject(
        selectedObject,
        state
    );
    Mesh* newMesh =
        nullptr;

    if (selectedObject->editorMeshType == "Sphere")
    {
        newMesh =
            CreateProceduralSphereMesh(
                state.sphereRings,
                state.sphereSectors
            );
    }
    else if (selectedObject->editorMeshType == "Cylinder")
    {
        newMesh =
            CreateProceduralCylinderMesh(
                state.cylinderSides
            );
    }
    else if (selectedObject->editorMeshType == "Cone")
    {
        newMesh =
            CreateProceduralConeMesh(
                state.coneSides
            );
    }

    if (newMesh == nullptr)
        return;

    // Do not delete the old mesh here.
    // Some meshes are shared between many objects.
    selectedObject->mesh =
        newMesh;

    selectedObject->boundingRadius =
        50.0f;
}

static void DrawPrimitiveMeshDetailControls(
    SceneObject* selectedObject
)
{
    if (!IsPrimitiveShapeObject(selectedObject))
        return;

    ImGui::Separator();

    if (!ImGui::CollapsingHeader("Primitive Mesh Detail V2"))
    {
        return;
    }

    ImGui::TextWrapped(
        "Regenerate the actual procedural mesh detail. "
        "Use low values for low-poly style and high values for smooth shapes."
    );

    ImGui::Spacing();

    PrimitiveMeshDetailState& state =
        GetPrimitiveMeshDetailState(
            selectedObject
        );

    bool changed =
        false;

    if (selectedObject->editorMeshType == "Sphere")
    {
        ImGui::Text("Mesh Type: Sphere / Ball");

        changed |=
            ImGui::SliderInt(
                "Rings##SphereRings",
                &state.sphereRings,
                4,
                64
            );

        changed |=
            ImGui::SliderInt(
                "Sectors##SphereSectors",
                &state.sphereSectors,
                6,
                96
            );

        if (ImGui::Button("Low Poly Sphere"))
        {
            state.sphereRings =
                8;

            state.sphereSectors =
                12;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Smooth Sphere"))
        {
            state.sphereRings =
                32;

            state.sphereSectors =
                48;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }
    }
    else if (selectedObject->editorMeshType == "Cylinder")
    {
        ImGui::Text("Mesh Type: Cylinder");

        changed |=
            ImGui::SliderInt(
                "Sides##CylinderSides",
                &state.cylinderSides,
                3,
                96
            );

        if (ImGui::Button("Triangle Column"))
        {
            state.cylinderSides =
                3;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Hex Column"))
        {
            state.cylinderSides =
                6;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }

        if (ImGui::Button("Smooth Cylinder"))
        {
            state.cylinderSides =
                48;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }
    }
    else if (selectedObject->editorMeshType == "Cone")
    {
        ImGui::Text("Mesh Type: Cone");

        changed |=
            ImGui::SliderInt(
                "Sides##ConeSides",
                &state.coneSides,
                3,
                96
            );

        if (ImGui::Button("Pyramid Cone"))
        {
            state.coneSides =
                4;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }

        ImGui::SameLine();

        if (ImGui::Button("Low Poly Cone"))
        {
            state.coneSides =
                8;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }

        if (ImGui::Button("Smooth Cone"))
        {
            state.coneSides =
                48;

            ApplyPrimitiveMeshDetail(
                selectedObject
            );
        }
    }

    ImGui::Spacing();

    if (
        changed ||
        ImGui::Button("Regenerate Mesh")
        )
    {
        ApplyPrimitiveMeshDetail(
            selectedObject
        );
    }
}
static SceneObject* SpawnProceduralPrimitive(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* mesh,
    Shader* shader,
    const std::string& objectName,
    const glm::vec3& scale,
    const glm::vec3& tint,
    float terrainOffset,
    float colliderRadius
)
{
    if (
        mesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    Material* objectMaterial =
        new Material(
            nullptr
        );

    objectMaterial->tint =
        tint;

    objectMaterial->ambient =
        tint * 0.45f;

    objectMaterial->diffuse =
        tint;

    objectMaterial->specular =
        glm::vec3(
            0.08f,
            0.08f,
            0.08f
        );

    objectMaterial->shininess =
        12.0f;

    SceneObject* object =
        new SceneObject(
            mesh,
            shader,
            objectMaterial
        );

    object->name =
        objectName;

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 6.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            terrainOffset
        );

    object->transform.position =
        spawnPosition;

    object->transform.rotation =
        glm::vec3(
            0.0f
        );

    object->transform.scale =
        scale;

    object->isCollider =
        true;

    object->colliderRadius =
        colliderRadius;

    object->boundingRadius =
        colliderRadius * 8.0f;
    SetEditorSaveMetadata(
        object,
        objectName.find("Ball") != std::string::npos ? "Sphere" :
        objectName.find("Cylinder") != std::string::npos ? "Cylinder" :
        objectName.find("Cone") != std::string::npos ? "Cone" :
        "Cube"
    );
    object->assetId =
        objectName;

    object->assetType =
        AssetType::Prop;

    object->spawnSource =
        SpawnSource::Manual;

    object->persistent =
        true;

    object->showInHierarchy =
        true;

    scene.AddObject(
        object
    );

    selectedObject =
        object;

    return object;
}
static SceneObject* SpawnStairsObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return nullptr;

    static Mesh* stairsMesh =
        nullptr;

    if (stairsMesh == nullptr)
    {
        stairsMesh =
            CreateProceduralStairsMesh(
                5
            );
    }

    Material* stairsMaterial =
        new Material(
            nullptr
        );

    stairsMaterial->tint =
        glm::vec3(
            0.55f,
            0.50f,
            0.44f
        );

    stairsMaterial->ambient =
        glm::vec3(
            0.32f,
            0.30f,
            0.27f
        );

    stairsMaterial->diffuse =
        glm::vec3(
            0.75f,
            0.70f,
            0.62f
        );

    stairsMaterial->specular =
        glm::vec3(
            0.04f
        );

    stairsMaterial->shininess =
        6.0f;

    SceneObject* stairs =
        new SceneObject(
            stairsMesh,
            shader,
            stairsMaterial
        );

    stairs->name =
        "Procedural Stairs";

    glm::vec3 spawnPosition =
        camera.Position +
        camera.Front * 6.0f;

    stairs->transform.position =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.50f
        );

    stairs->transform.rotation =
        glm::vec3(
            0.0f
        );

    stairs->transform.scale =
        glm::vec3(
            4.0f,
            2.0f,
            4.0f
        );

    stairs->visible =
        true;

    stairs->isCollider =
        true;

    stairs->colliderRadius =
        4.0f;

    stairs->boundingRadius =
        50.0f;

    SetEditorSaveMetadata(
        stairs,
        "Stairs",
        "None"
    );

    stairs->assetId =
        "Procedural Stairs";

    stairs->assetType =
        AssetType::Prop;

    stairs->spawnSource =
        SpawnSource::Manual;

    stairs->persistent =
        true;

    stairs->showInHierarchy =
        true;

    scene.AddObject(
        stairs
    );

    selectedObject =
        stairs;

    return stairs;
}
static SceneObject* SpawnRingObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return nullptr;

    static Mesh* ringMesh =
        nullptr;

    if (ringMesh == nullptr)
    {
        ringMesh =
            CreateProceduralRingMesh(
                32,
                12
            );
    }

    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(
            0.75f,
            0.62f,
            0.25f
        );

    material->ambient =
        glm::vec3(
            0.35f,
            0.28f,
            0.12f
        );

    material->diffuse =
        glm::vec3(
            0.85f,
            0.70f,
            0.30f
        );

    material->specular =
        glm::vec3(
            0.30f,
            0.25f,
            0.10f
        );

    material->shininess =
        32.0f;

    SceneObject* ring =
        new SceneObject(
            ringMesh,
            shader,
            material
        );

    ring->name =
        "Procedural Ring";

    glm::vec3 spawnPosition =
        camera.Position +
        camera.Front * 6.0f;

    ring->transform.position =
        SnapEditorPositionToTerrain(
            spawnPosition,
            1.0f
        );

    ring->transform.rotation =
        glm::vec3(
            90.0f,
            0.0f,
            0.0f
        );

    ring->transform.scale =
        glm::vec3(
            2.0f,
            2.0f,
            2.0f
        );

    ring->visible =
        true;

    ring->isCollider =
        false;

    ring->colliderRadius =
        2.0f;

    ring->boundingRadius =
        50.0f;

    SetEditorSaveMetadata(
        ring,
        "Ring",
        "None"
    );

    ring->assetId =
        "Procedural Ring";

    ring->assetType =
        AssetType::Prop;

    ring->spawnSource =
        SpawnSource::Manual;

    ring->persistent =
        true;

    ring->showInHierarchy =
        true;

    scene.AddObject(
        ring
    );

    selectedObject =
        ring;

    return ring;
}

static SceneObject* SpawnPipeObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return nullptr;

    static Mesh* pipeMesh =
        nullptr;

    if (pipeMesh == nullptr)
    {
        pipeMesh =
            CreateProceduralPipeMesh(
                32
            );
    }

    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(
            0.42f,
            0.42f,
            0.45f
        );

    material->ambient =
        glm::vec3(
            0.25f,
            0.25f,
            0.27f
        );

    material->diffuse =
        glm::vec3(
            0.65f,
            0.65f,
            0.68f
        );

    material->specular =
        glm::vec3(
            0.25f,
            0.25f,
            0.28f
        );

    material->shininess =
        20.0f;

    SceneObject* pipe =
        new SceneObject(
            pipeMesh,
            shader,
            material
        );

    pipe->name =
        "Procedural Pipe";

    glm::vec3 spawnPosition =
        camera.Position +
        camera.Front * 6.0f;

    pipe->transform.position =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.60f
        );

    pipe->transform.rotation =
        glm::vec3(
            0.0f
        );

    pipe->transform.scale =
        glm::vec3(
            1.5f,
            2.5f,
            1.5f
        );

    pipe->visible =
        true;

    pipe->isCollider =
        true;

    pipe->colliderRadius =
        2.0f;

    pipe->boundingRadius =
        50.0f;

    SetEditorSaveMetadata(
        pipe,
        "Pipe",
        "None"
    );

    pipe->assetId =
        "Procedural Pipe";

    pipe->assetType =
        AssetType::Prop;

    pipe->spawnSource =
        SpawnSource::Manual;

    pipe->persistent =
        true;

    pipe->showInHierarchy =
        true;

    scene.AddObject(
        pipe
    );

    selectedObject =
        pipe;

    return pipe;
}
static SceneObject* SpawnCapsuleObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return nullptr;

    static Mesh* capsuleMesh =
        nullptr;

    if (capsuleMesh == nullptr)
    {
        capsuleMesh =
            CreateProceduralCapsuleMesh(
                12,
                32
            );
    }

    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(
            0.40f,
            0.62f,
            0.85f
        );

    material->ambient =
        glm::vec3(
            0.18f,
            0.28f,
            0.40f
        );

    material->diffuse =
        glm::vec3(
            0.55f,
            0.75f,
            0.95f
        );

    material->specular =
        glm::vec3(
            0.20f,
            0.30f,
            0.40f
        );

    material->shininess =
        24.0f;

    SceneObject* capsule =
        new SceneObject(
            capsuleMesh,
            shader,
            material
        );

    capsule->name =
        "Procedural Capsule";

    glm::vec3 spawnPosition =
        camera.Position +
        camera.Front * 6.0f;

    capsule->transform.position =
        SnapEditorPositionToTerrain(
            spawnPosition,
            1.0f
        );

    capsule->transform.rotation =
        glm::vec3(
            0.0f
        );

    capsule->transform.scale =
        glm::vec3(
            1.4f,
            2.5f,
            1.4f
        );

    capsule->visible =
        true;

    capsule->isCollider =
        true;

    capsule->colliderRadius =
        1.5f;

    capsule->boundingRadius =
        50.0f;

    SetEditorSaveMetadata(
        capsule,
        "Capsule",
        "None"
    );

    capsule->assetId =
        "Procedural Capsule";

    capsule->assetType =
        AssetType::Prop;

    capsule->spawnSource =
        SpawnSource::Manual;

    capsule->persistent =
        true;

    capsule->showInHierarchy =
        true;

    scene.AddObject(
        capsule
    );

    selectedObject =
        capsule;

    return capsule;
}

static SceneObject* SpawnRoofWedgeObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return nullptr;

    static Mesh* roofWedgeMesh =
        nullptr;

    if (roofWedgeMesh == nullptr)
    {
        roofWedgeMesh =
            CreateProceduralRoofWedgeMesh();
    }

    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(
            0.50f,
            0.18f,
            0.10f
        );

    material->ambient =
        glm::vec3(
            0.25f,
            0.10f,
            0.06f
        );

    material->diffuse =
        glm::vec3(
            0.70f,
            0.28f,
            0.16f
        );

    material->specular =
        glm::vec3(
            0.05f
        );

    material->shininess =
        6.0f;

    SceneObject* roof =
        new SceneObject(
            roofWedgeMesh,
            shader,
            material
        );

    roof->name =
        "Procedural Roof Wedge";

    glm::vec3 spawnPosition =
        camera.Position +
        camera.Front * 6.0f;

    roof->transform.position =
        SnapEditorPositionToTerrain(
            spawnPosition,
            1.0f
        );

    roof->transform.rotation =
        glm::vec3(
            0.0f
        );

    roof->transform.scale =
        glm::vec3(
            5.0f,
            1.8f,
            4.0f
        );

    roof->visible =
        true;

    roof->isCollider =
        true;

    roof->colliderRadius =
        4.0f;

    roof->boundingRadius =
        50.0f;

    SetEditorSaveMetadata(
        roof,
        "RoofWedge",
        "None"
    );

    roof->assetId =
        "Procedural Roof Wedge";

    roof->assetType =
        AssetType::Prop;

    roof->spawnSource =
        SpawnSource::Manual;

    roof->persistent =
        true;

    roof->showInHierarchy =
        true;

    scene.AddObject(
        roof
    );

    selectedObject =
        roof;

    return roof;
}
static SceneObject* CreateArchPiece(
    Scene& scene,
    Mesh* cubeMesh,
    Shader* shader,
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& scale
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(
            0.46f,
            0.43f,
            0.38f
        );

    material->ambient =
        glm::vec3(
            0.30f,
            0.28f,
            0.25f
        );

    material->diffuse =
        glm::vec3(
            0.70f,
            0.66f,
            0.58f
        );

    material->specular =
        glm::vec3(
            0.04f
        );

    material->shininess =
        5.0f;

    SceneObject* piece =
        new SceneObject(
            cubeMesh,
            shader,
            material
        );

    piece->name =
        name;

    piece->transform.position =
        position;

    piece->transform.rotation =
        glm::vec3(
            0.0f
        );

    piece->transform.scale =
        scale;

    piece->visible =
        true;

    piece->isCollider =
        true;

    piece->colliderRadius =
        2.0f;

    piece->boundingRadius =
        40.0f;

    SetEditorSaveMetadata(
        piece,
        "Cube",
        "None"
    );

    piece->assetId =
        name;

    piece->assetType =
        AssetType::Prop;

    piece->spawnSource =
        SpawnSource::Manual;

    piece->persistent =
        true;

    piece->showInHierarchy =
        true;

    scene.AddObject(
        piece
    );

    return piece;
}

static SceneObject* SpawnArchObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 basePosition =
        camera.Position +
        forward * 7.0f;

    glm::vec3 center =
        SnapEditorPositionToTerrain(
            basePosition,
            1.50f
        );

    SceneObject* leftPillar =
        CreateArchPiece(
            scene,
            cubeMesh,
            shader,
            "Arch Left Pillar",
            center + glm::vec3(
                -2.0f,
                0.0f,
                0.0f
            ),
            glm::vec3(
                0.45f,
                3.0f,
                0.55f
            )
        );

    CreateArchPiece(
        scene,
        cubeMesh,
        shader,
        "Arch Right Pillar",
        center + glm::vec3(
            2.0f,
            0.0f,
            0.0f
        ),
        glm::vec3(
            0.45f,
            3.0f,
            0.55f
        )
    );

    CreateArchPiece(
        scene,
        cubeMesh,
        shader,
        "Arch Top Beam",
        center + glm::vec3(
            0.0f,
            1.55f,
            0.0f
        ),
        glm::vec3(
            4.5f,
            0.45f,
            0.55f
        )
    );

    selectedObject =
        leftPillar;

    return leftPillar;
}
static void PlaceCoinOnTerrain(
    SceneObject* coin,
    float heightOffset = 0.45f
)
{
    if (coin == nullptr)
        return;

    coin->transform.position.y =
        GetTerrainHeight(
            coin->transform.position.x,
            coin->transform.position.z
        ) + heightOffset;
}
static SceneObject* SpawnCoinObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    Mesh* coinMesh =
        GetProceduralPrimitiveMesh(
            "Cylinder"
        );

    if (
        coinMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    Material* coinMaterial =
        new Material(
            nullptr
        );

    coinMaterial->tint =
        glm::vec3(
            1.0f,
            0.82f,
            0.12f
        );

    coinMaterial->ambient =
        glm::vec3(
            0.55f,
            0.40f,
            0.08f
        );

    coinMaterial->diffuse =
        glm::vec3(
            1.0f,
            0.78f,
            0.12f
        );

    coinMaterial->specular =
        glm::vec3(
            0.85f,
            0.70f,
            0.25f
        );

    coinMaterial->shininess =
        48.0f;

    SceneObject* coin =
        new SceneObject(
            coinMesh,
            shader,
            coinMaterial
        );

    coin->name =
        "Coin";

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 6.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.18f
        );
    coin->transform.position =
        spawnPosition;
    PlaceCoinOnTerrain(
        coin,
        0.18f
    );
    coin->transform.rotation =
        glm::vec3(
            0.0f,
            0.0f,
            0.0f
        );

    coin->transform.scale =
        glm::vec3(
            0.55f,
            0.10f,
            0.55f
        );

    coin->visible =
        true;

    coin->isCollider =
        false;

    coin->colliderRadius =
        1.0f;

    coin->boundingRadius =
        10.0f;

    SetEditorSaveMetadata(
        coin,
        "Cylinder",
        "Coin"
    );

    coin->assetId =
        "Coin";

    coin->assetType =
        AssetType::Prop;

    coin->spawnSource =
        SpawnSource::Manual;

    coin->persistent =
        true;

    coin->showInHierarchy =
        true;

    scene.AddObject(
        coin
    );

    selectedObject =
        coin;

    std::cout
        << "Coin spawned."
        << std::endl;

    return coin;
}
static SceneObject* SpawnTriggerZoneObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    Material* triggerMaterial =
        new Material(
            nullptr
        );

    triggerMaterial->tint =
        glm::vec3(
            0.20f,
            0.45f,
            1.0f
        );

    triggerMaterial->ambient =
        glm::vec3(
            0.05f,
            0.12f,
            0.30f
        );

    triggerMaterial->diffuse =
        glm::vec3(
            0.20f,
            0.45f,
            1.0f
        );

    triggerMaterial->specular =
        glm::vec3(
            0.30f,
            0.50f,
            1.0f
        );

    triggerMaterial->shininess =
        24.0f;

    triggerMaterial->wireframe =
        true;

    SceneObject* trigger =
        new SceneObject(
            cubeMesh,
            shader,
            triggerMaterial
        );

    trigger->name =
        "Trigger Zone";

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 7.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.2f
        );

    trigger->transform.position =
        spawnPosition;

    trigger->transform.rotation =
        glm::vec3(
            0.0f
        );

    trigger->transform.scale =
        glm::vec3(
            6.0f,
            0.15f,
            6.0f
        );

    trigger->visible =
        true;

    trigger->isCollider =
        false;

    trigger->colliderRadius =
        3.5f;

    trigger->boundingRadius =
        30.0f;

    SetEditorSaveMetadata(
        trigger,
        "Cube",
        "TriggerZone"
    );

    trigger->assetId =
        "Trigger Zone";

    trigger->assetType =
        AssetType::Prop;

    trigger->spawnSource =
        SpawnSource::Manual;

    trigger->persistent =
        true;

    trigger->showInHierarchy =
        true;

    scene.AddObject(
        trigger
    );

    selectedObject =
        trigger;

    std::cout
        << "Trigger Zone spawned."
        << std::endl;

    return trigger;
}

static SceneObject* SpawnMonsterSpawnObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
    {
        return nullptr;
    }

    static Model* monsterModel =
        nullptr;

    static std::string loadedMonsterPath =
        "";

    std::string monsterRelativePath =
        "Assets/Models/Environment/Campfire/monster_test.obj";

    /*
        TEST ORDER:

        1. First test with:
           Assets/Models/Environment/Campfire/campfire.obj

        2. If that works, copy monster_test.obj into:
           Assets/Models/Environment/Campfire/

           Then change monsterRelativePath to:
           Assets/Models/Environment/Campfire/monster_test.obj

        3. If OBJ works, copy monster_test.fbx into:
           Assets/Models/Environment/Campfire/

           Then change monsterRelativePath to:
           Assets/Models/Environment/Campfire/monster_test.fbx
    */

    std::string monsterPath =
        ResolveAssetPath(
            monsterRelativePath
        );

    std::string monsterDirectory =
        GetDirectoryFromPath(
            monsterPath
        );

    std::cout
        << "Trying to load monster test model: "
        << monsterPath
        << std::endl;

    std::cout
        << "Monster test file exists: "
        << FileExists(
            monsterPath
        )
        << std::endl;

    if (
        monsterModel == nullptr ||
        loadedMonsterPath != monsterPath
        )
    {
        monsterModel =
            new Model(
                monsterPath,
                monsterDirectory
            );

        loadedMonsterPath =
            monsterPath;
    }

    SceneObject* monster =
        new SceneObject(
            monsterModel,
            shader
        );

    monster->name =
        "Monster Spawn";

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 10.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            2.5f
        );

    monster->transform.position =
        spawnPosition;

    monster->transform.rotation =
        glm::vec3(
            0.0f,
            180.0f,
            0.0f
        );

    monster->transform.scale =
        glm::vec3(
            0.30f
        );

    monster->visible =
        true;

    monster->isCollider =
        true;

    monster->colliderRadius =
        1.7f;

    monster->boundingRadius =
        35.0f;

    SetEditorSaveMetadata(
        monster,
        "Model",
        "MonsterSpawn",
        monsterRelativePath,
        "Assets/Models/Environment/Campfire/"
    );

    monster->assetId =
        "Monster Spawn";

    monster->assetType =
        AssetType::Prop;

    monster->spawnSource =
        SpawnSource::Manual;

    monster->persistent =
        true;

    monster->showInHierarchy =
        true;

    scene.AddObject(
        monster
    );

    selectedObject =
        monster;

    std::cout
        << "Monster test spawn placed."
        << std::endl;

    return monster;
}
static Material* CreateMusicGateMaterial(
    const glm::vec3& color,
    bool wireframe
)
{
    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        color;

    material->ambient =
        color * 0.45f;

    material->diffuse =
        color;

    material->specular =
        glm::vec3(
            0.45f,
            0.75f,
            1.0f
        );

    material->shininess =
        64.0f;

    material->wireframe =
        wireframe;

    return material;
}

static SceneObject* CreateMusicGateVisualPiece(
    Scene& scene,
    Mesh* cubeMesh,
    Shader* shader,
    const std::string& name,
    const glm::vec3& position,
    const glm::vec3& scale,
    const glm::vec3& color,
    bool wireframe
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    SceneObject* piece =
        new SceneObject(
            cubeMesh,
            shader,
            CreateMusicGateMaterial(
                color,
                wireframe
            )
        );

    piece->name =
        name;

    piece->transform.position =
        position;

    piece->transform.rotation =
        glm::vec3(
            0.0f
        );

    piece->transform.scale =
        scale;

    piece->visible =
        true;

    piece->isCollider =
        false;

    piece->colliderRadius =
        1.0f;

    piece->boundingRadius =
        40.0f;

    SetEditorSaveMetadata(
        piece,
        "Cube",
        "None"
    );

    piece->assetId =
        name;

    piece->assetType =
        AssetType::Prop;

    piece->spawnSource =
        SpawnSource::Manual;

    piece->persistent =
        true;

    piece->showInHierarchy =
        true;

    scene.AddObject(
        piece
    );

    return piece;
}
static SceneObject* SpawnMusicGateObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return nullptr;
    }

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                )
            )
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 8.0f;

    glm::vec3 groundPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.10f
        );

    glm::vec3 gateCenter =
        SnapEditorPositionToTerrain(
            spawnPosition,
            1.70f
        );

    glm::vec3 portalBlue =
        glm::vec3(
            0.10f,
            0.55f,
            1.0f
        );

    glm::vec3 portalDarkBlue =
        glm::vec3(
            0.02f,
            0.14f,
            0.35f
        );

    glm::vec3 portalGlow =
        glm::vec3(
            0.25f,
            0.85f,
            1.0f
        );

    SceneObject* gate =
        new SceneObject(
            cubeMesh,
            shader,
            CreateMusicGateMaterial(
                portalGlow,
                true
            )
        );

    gate->name =
        "Music Gate";

    gate->transform.position =
        gateCenter;

    gate->transform.rotation =
        glm::vec3(
            0.0f
        );

    gate->transform.scale =
        glm::vec3(
            6.2f,
            3.4f,
            0.35f
        );

    gate->visible =
        true;

    gate->isCollider =
        false;

    gate->colliderRadius =
        4.5f;

    gate->boundingRadius =
        45.0f;

    SetEditorSaveMetadata(
        gate,
        "Cube",
        "MusicGate"
    );

    gate->assetId =
        "Music Gate";

    gate->assetType =
        AssetType::Gameplay;

    gate->spawnSource =
        SpawnSource::Manual;

    gate->persistent =
        true;

    gate->showInHierarchy =
        true;

    scene.AddObject(
        gate
    );

    CreateMusicGateVisualPiece(
        scene,
        cubeMesh,
        shader,
        "Rescue Portal Left Pillar",
        gateCenter - right * 3.0f,
        glm::vec3(
            0.35f,
            3.4f,
            0.35f
        ),
        portalBlue,
        false
    );

    CreateMusicGateVisualPiece(
        scene,
        cubeMesh,
        shader,
        "Rescue Portal Right Pillar",
        gateCenter + right * 3.0f,
        glm::vec3(
            0.35f,
            3.4f,
            0.35f
        ),
        portalBlue,
        false
    );

    CreateMusicGateVisualPiece(
        scene,
        cubeMesh,
        shader,
        "Rescue Portal Top Beam",
        gateCenter +
        glm::vec3(
            0.0f,
            1.75f,
            0.0f
        ),
        glm::vec3(
            6.4f,
            0.35f,
            0.35f
        ),
        portalGlow,
        false
    );

    CreateMusicGateVisualPiece(
        scene,
        cubeMesh,
        shader,
        "Rescue Portal Base Platform",
        groundPosition,
        glm::vec3(
            6.8f,
            0.12f,
            2.0f
        ),
        portalDarkBlue,
        false
    );

    selectedObject =
        gate;

    std::cout
        << "Music Gate rescue portal spawned."
        << std::endl;

    return gate;
}

static SceneObject* SpawnMusicNpcObject(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
    {
        return nullptr;
    }

    static Model* musicNpcModel =
        nullptr;

    static std::string loadedMusicNpcPath =
        "";

    std::string musicNpcRelativePath =
        "Assets/Models/Charachters/Player/music_npc.fbx";

    std::string musicNpcPath =
        ResolveAssetPath(
            musicNpcRelativePath
        );

    std::string musicNpcDirectory =
        GetDirectoryFromPath(
            musicNpcPath
        );

    std::cout
        << "Trying to load Music NPC model: "
        << musicNpcPath
        << std::endl;

    std::cout
        << "Music NPC file exists: "
        << FileExists(
            musicNpcPath
        )
        << std::endl;

    if (
        musicNpcModel == nullptr ||
        loadedMusicNpcPath != musicNpcPath
        )
    {
        musicNpcModel =
            new Model(
                musicNpcPath,
                musicNpcDirectory
            );

        loadedMusicNpcPath =
            musicNpcPath;
    }

    SceneObject* musicNpc =
        new SceneObject(
            musicNpcModel,
            shader
        );

    musicNpc->name =
        "Music NPC";

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                )
            )
        );

    glm::vec3 spawnPosition =
        camera.Position +
        forward * 7.0f +
        right * 2.0f;

    spawnPosition =
        SnapEditorPositionToTerrain(
            spawnPosition,
            0.15f
        );

    musicNpc->transform.position =
        spawnPosition;

    musicNpc->transform.rotation =
        glm::vec3(
            90.0f,
            180.0f,
            0.0f
        );

    musicNpc->transform.scale =
        glm::vec3(
            1.60f
        );

    musicNpc->visible =
        true;

    musicNpc->isCollider =
        false;

    musicNpc->colliderRadius =
        1.2f;

    musicNpc->boundingRadius =
        25.0f;

    SetEditorSaveMetadata(
        musicNpc,
        "Model",
        "MusicNPC",
        musicNpcRelativePath,
        "Assets/Models/Charachters/Player/"
    );

    musicNpc->assetId =
        "Music NPC";

    musicNpc->assetType =
        AssetType::Prop;

    musicNpc->spawnSource =
        SpawnSource::Manual;

    musicNpc->persistent =
        true;

    musicNpc->showInHierarchy =
        true;

    scene.AddObject(
        musicNpc
    );

    selectedObject =
        musicNpc;

    std::cout
        << "Music NPC spawned."
        << std::endl;

    return musicNpc;
}
static void BuildDemoScene(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return;
    }

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                )
            )
        );

    glm::vec3 basePosition =
        camera.Position +
        forward * 10.0f;

    basePosition =
        SnapEditorPositionToTerrain(
            basePosition,
            0.0f
        );

    // ================= TRIGGER ZONE =================
    SceneObject* trigger =
        SpawnTriggerZoneObject(
            scene,
            selectedObject,
            camera,
            cubeMesh,
            shader
        );

    if (trigger != nullptr)
    {
        trigger->name =
            "Demo Trigger Zone";

        trigger->transform.position =
            SnapEditorPositionToTerrain(
                basePosition,
                0.12f
            );

        trigger->transform.scale =
            glm::vec3(
                6.0f,
                0.15f,
                6.0f
            );

        trigger->editorGameplayType =
            "TriggerZone";
    }

    // ================= MONSTER SPAWN =================
    SceneObject* monster =
        SpawnMonsterSpawnObject(
            scene,
            selectedObject,
            camera,
            shader
        );

    if (monster != nullptr)
    {
        monster->name =
            "Demo Monster Spawn";

        monster->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                forward * 18.0f,
                2.0f
            );

        monster->editorGameplayType =
            "MonsterSpawn";
    }

    // ================= MUSIC GATE =================
    SceneObject* musicGate =
        SpawnMusicGateObject(
            scene,
            selectedObject,
            camera,
            cubeMesh,
            shader
        );

    if (musicGate != nullptr)
    {
        musicGate->name =
            "Music Gate";

        musicGate->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                right * 14.0f +
                forward * 6.0f,
                1.70f
            );

        musicGate->editorGameplayType =
            "MusicGate";
    }

    // ================= MUSIC NPC =================
    SceneObject* musicNpc =
        SpawnMusicNpcObject(
            scene,
            selectedObject,
            camera,
            shader
        );

    if (musicNpc != nullptr)
    {
        musicNpc->name =
            "Demo Music NPC";

        musicNpc->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                right * 17.0f +
                forward * 6.0f,
                0.15f
            );

        musicNpc->transform.rotation =
            glm::vec3(
                90.0f,
                180.0f,
                0.0f
            );

        musicNpc->editorGameplayType =
            "MusicNPC";
    }

    // ================= COINS =================
    for (int i = 0; i < 10; i++)
    {
        SceneObject* coin =
            SpawnCoinObject(
                scene,
                selectedObject,
                camera,
                shader
            );

        if (coin != nullptr)
        {
            float row =
                (float)(i / 5);

            float column =
                (float)(i % 5);

            coin->name =
                "Demo Coin " +
                std::to_string(
                    i + 1
                );

            coin->transform.position =
                basePosition +
                forward * (4.0f + row * 3.0f) +
                right * ((column - 2.0f) * 2.2f);

            PlaceCoinOnTerrain(
                coin,
                0.18f
            );

            coin->editorGameplayType =
                "Coin";
        }
    }

    selectedObject =
        musicGate;

    std::cout
        << "Demo scene built successfully."
        << std::endl;
}
// ================= GAMEPLAY PRESET BUILDER V1 =================

static void GetGameplayPresetDirections(
    Camera& camera,
    glm::vec3& forward,
    glm::vec3& right
)
{
    forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    right =
        glm::normalize(
            glm::cross(
                forward,
                glm::vec3(
                    0.0f,
                    1.0f,
                    0.0f
                )
            )
        );
}

static void BuildCoinPathPreset(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Shader* shader
)
{
    if (shader == nullptr)
        return;

    glm::vec3 forward;
    glm::vec3 right;

    GetGameplayPresetDirections(
        camera,
        forward,
        right
    );

    glm::vec3 startPosition =
        camera.Position +
        forward * 8.0f;

    startPosition =
        SnapEditorPositionToTerrain(
            startPosition,
            0.18f
        );

    for (int i = 0; i < 15; i++)
    {
        SceneObject* coin =
            SpawnCoinObject(
                scene,
                selectedObject,
                camera,
                shader
            );

        if (coin == nullptr)
            continue;

        float sideOffset =
            std::sin((float)i * 0.75f) *
            2.0f;

        coin->name =
            "Coin Path Coin " +
            std::to_string(i + 1);

        coin->transform.position =
            startPosition +
            forward * ((float)i * 2.8f) +
            right * sideOffset;

        PlaceCoinOnTerrain(
            coin,
            0.18f
        );

        coin->editorGameplayType =
            "Coin";

        coin->persistent =
            true;

        coin->showInHierarchy =
            true;

        selectedObject =
            coin;
    }

    std::cout
        << "Coin path preset created."
        << std::endl;
}

static void BuildMonsterEncounterPreset(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return;
    }

    glm::vec3 forward;
    glm::vec3 right;

    GetGameplayPresetDirections(
        camera,
        forward,
        right
    );

    glm::vec3 basePosition =
        camera.Position +
        forward * 10.0f;

    basePosition =
        SnapEditorPositionToTerrain(
            basePosition,
            0.12f
        );

    SceneObject* trigger =
        SpawnTriggerZoneObject(
            scene,
            selectedObject,
            camera,
            cubeMesh,
            shader
        );

    if (trigger != nullptr)
    {
        trigger->name =
            "Monster Encounter Trigger";

        trigger->transform.position =
            SnapEditorPositionToTerrain(
                basePosition,
                0.12f
            );

        trigger->transform.scale =
            glm::vec3(
                7.0f,
                0.15f,
                7.0f
            );

        trigger->editorGameplayType =
            "TriggerZone";

        trigger->persistent =
            true;
    }

    SceneObject* monster =
        SpawnMonsterSpawnObject(
            scene,
            selectedObject,
            camera,
            shader
        );

    if (monster != nullptr)
    {
        monster->name =
            "Monster Encounter Spawn";

        monster->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                forward * 16.0f,
                2.0f
            );

        monster->editorGameplayType =
            "MonsterSpawn";

        monster->persistent =
            true;

        selectedObject =
            monster;
    }

    std::cout
        << "Monster encounter preset created."
        << std::endl;
}

static void BuildMusicRescuePreset(
    Scene& scene,
    SceneObject*& selectedObject,
    Camera& camera,
    Mesh* cubeMesh,
    Shader* shader
)
{
    if (
        cubeMesh == nullptr ||
        shader == nullptr
        )
    {
        return;
    }

    glm::vec3 forward;
    glm::vec3 right;

    GetGameplayPresetDirections(
        camera,
        forward,
        right
    );

    glm::vec3 basePosition =
        camera.Position +
        forward * 10.0f;

    basePosition =
        SnapEditorPositionToTerrain(
            basePosition,
            0.0f
        );

    SceneObject* gate =
        SpawnMusicGateObject(
            scene,
            selectedObject,
            camera,
            cubeMesh,
            shader
        );

    if (gate != nullptr)
    {
        gate->name =
            "Music Rescue Gate";

        gate->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                right * 8.0f,
                1.70f
            );

        gate->editorGameplayType =
            "MusicGate";

        gate->persistent =
            true;
    }

    SceneObject* npc =
        SpawnMusicNpcObject(
            scene,
            selectedObject,
            camera,
            shader
        );

    if (npc != nullptr)
    {
        npc->name =
            "Music Rescue NPC";

        npc->transform.position =
            SnapEditorPositionToTerrain(
                basePosition +
                right * 11.0f,
                0.15f
            );

        npc->transform.rotation =
            glm::vec3(
                90.0f,
                180.0f,
                0.0f
            );

        npc->editorGameplayType =
            "MusicNPC";

        npc->persistent =
            true;

        selectedObject =
            npc;
    }

    std::cout
        << "Music rescue preset created."
        << std::endl;
}
// ================= WORLD PAINTER V1 + V2 =================

static float WorldPainterRandomFloat(
    float minValue,
    float maxValue
)
{
    float t =
        static_cast<float>(
            std::rand()
            ) /
        static_cast<float>(
            RAND_MAX
            );

    return
        minValue +
        (maxValue - minValue) *
        t;
}

static glm::vec3 GetWorldPainterCenter(
    Camera& camera,
    float distance
)
{
    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 center =
        camera.Position +
        forward *
        distance;

    center =
        SnapEditorPositionToTerrain(
            center,
            0.15f
        );

    return center;
}

static void AddWorldPainterVertex(
    std::vector<float>& data,
    const glm::vec3& position,
    const glm::vec3& normal,
    const glm::vec2& texCoord
)
{
    data.push_back(position.x);
    data.push_back(position.y);
    data.push_back(position.z);

    data.push_back(normal.x);
    data.push_back(normal.y);
    data.push_back(normal.z);

    data.push_back(texCoord.x);
    data.push_back(texCoord.y);
}

static Mesh* CreateWorldPainterPreviewMesh()
{
    std::vector<float> data;

    const float pi =
        3.14159265359f;

    int segments =
        96;

    float outerRadius =
        1.0f;

    float innerRadius =
        0.94f;

    glm::vec3 normal =
        glm::vec3(
            0.0f,
            1.0f,
            0.0f
        );

    for (int i = 0; i < segments; i++)
    {
        float a0 =
            static_cast<float>(i) /
            static_cast<float>(segments) *
            pi *
            2.0f;

        float a1 =
            static_cast<float>(i + 1) /
            static_cast<float>(segments) *
            pi *
            2.0f;

        glm::vec3 outer0 =
            glm::vec3(
                std::cos(a0) * outerRadius,
                0.0f,
                std::sin(a0) * outerRadius
            );

        glm::vec3 outer1 =
            glm::vec3(
                std::cos(a1) * outerRadius,
                0.0f,
                std::sin(a1) * outerRadius
            );

        glm::vec3 inner0 =
            glm::vec3(
                std::cos(a0) * innerRadius,
                0.0f,
                std::sin(a0) * innerRadius
            );

        glm::vec3 inner1 =
            glm::vec3(
                std::cos(a1) * innerRadius,
                0.0f,
                std::sin(a1) * innerRadius
            );

        AddWorldPainterVertex(
            data,
            inner0,
            normal,
            glm::vec2(0.0f, 0.0f)
        );

        AddWorldPainterVertex(
            data,
            outer0,
            normal,
            glm::vec2(1.0f, 0.0f)
        );

        AddWorldPainterVertex(
            data,
            outer1,
            normal,
            glm::vec2(1.0f, 1.0f)
        );

        AddWorldPainterVertex(
            data,
            inner0,
            normal,
            glm::vec2(0.0f, 0.0f)
        );

        AddWorldPainterVertex(
            data,
            outer1,
            normal,
            glm::vec2(1.0f, 1.0f)
        );

        AddWorldPainterVertex(
            data,
            inner1,
            normal,
            glm::vec2(0.0f, 1.0f)
        );
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() *
            sizeof(float)
            )
    );
}

static Mesh* GetWorldPainterPreviewMesh()
{
    static Mesh* previewMesh =
        nullptr;

    if (previewMesh == nullptr)
    {
        previewMesh =
            CreateWorldPainterPreviewMesh();
    }

    return previewMesh;
}

static SceneObject* FindWorldPainterPreview(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (object->name == "World Painter Preview")
            return object;
    }

    return nullptr;
}

static void UpdateWorldPainterPreview(
    Scene& scene,
    Shader* shader,
    Camera& camera,
    float brushRadius,
    float paintDistance,
    bool visible
)
{
    SceneObject* preview =
        FindWorldPainterPreview(
            scene
        );

    if (preview == nullptr)
    {
        Material* previewMaterial =
            new Material(
                nullptr
            );

        previewMaterial->tint =
            glm::vec3(
                0.0f,
                0.85f,
                1.0f
            );

        previewMaterial->ambient =
            glm::vec3(
                0.0f,
                0.45f,
                0.65f
            );

        previewMaterial->diffuse =
            glm::vec3(
                0.0f,
                0.85f,
                1.0f
            );

        previewMaterial->specular =
            glm::vec3(
                0.1f,
                0.4f,
                0.7f
            );

        previewMaterial->shininess =
            32.0f;

        previewMaterial->wireframe =
            true;

        preview =
            new SceneObject(
                GetWorldPainterPreviewMesh(),
                shader,
                previewMaterial
            );

        preview->name =
            "World Painter Preview";

        preview->assetId =
            "World Painter Preview";

        preview->assetType =
            AssetType::Prop;
        preview->spawnSource =
            SpawnSource::Procedural;

        preview->persistent =
            false;

        preview->showInHierarchy =
            false;

        preview->isCollider =
            false;

        preview->boundingRadius =
            200.0f;

        scene.AddObject(
            preview
        );
    }

    preview->visible =
        visible;

    if (!visible)
        return;

    glm::vec3 center =
        GetWorldPainterCenter(
            camera,
            paintDistance
        );
    center.y =
        GetTerrainHeight(
            center.x,
            center.z
        ) +
        1.5f +
        brushRadius * 0.10f;

    preview->transform.position =
        center;

    preview->transform.scale =
        glm::vec3(
            brushRadius,
            1.0f,
            brushRadius
        );

    preview->transform.rotation =
        glm::vec3(
            0.0f
        );
}

static void SpawnWorldPaintedModel(
    Scene& scene,
    SceneObject*& selectedObject,
    Shader* shader,
    Model* model,
    const std::string& objectName,
    const std::string& modelPath,
    const glm::vec3& position,
    float scaleValue,
    bool collider,
    bool randomRotation
)
{
    if (model == nullptr)
        return;

    SceneObject* object =
        new SceneObject(
            model,
            shader
        );

    object->name =
        objectName;

    object->transform.position =
        SnapEditorPositionToTerrain(
            position,
            2.0f
        );

    object->transform.scale =
        glm::vec3(
            scaleValue
        );

    if (randomRotation)
    {
        object->transform.rotation =
            glm::vec3(
                0.0f,
                WorldPainterRandomFloat(
                    0.0f,
                    360.0f
                ),
                0.0f
            );
    }

    object->isCollider =
        collider;

    object->colliderRadius =
        scaleValue *
        0.8f;

    object->boundingRadius =
        60.0f;

    object->assetId =
        objectName;

    object->assetType =
        AssetType::Prop;

    object->spawnSource =
        SpawnSource::Manual;

    object->persistent =
        true;

    object->showInHierarchy =
        true;

    SetEditorSaveMetadata(
        object,
        "Model",
        "None",
        modelPath,
        ""
    );

    scene.AddObject(
        object
    );

    selectedObject =
        object;
}

static void PaintWorldBrush(
    Scene& scene,
    SceneObject*& selectedObject,
    Shader* shader,
    Camera& camera,
    int brushType,
    float brushRadius,
    int density,
    float paintDistance,
    float minScale,
    float maxScale,
    bool randomRotation,
    Model* pineTreeModel,
    Model* rockModel,
    Model* grassModel,
    Model* bushModel
)
{
    glm::vec3 center =
        GetWorldPainterCenter(
            camera,
            paintDistance
        );

    for (int i = 0; i < density; i++)
    {
        float angle =
            WorldPainterRandomFloat(
                0.0f,
                6.28318530718f
            );

        float radius =
            std::sqrt(
                WorldPainterRandomFloat(
                    0.0f,
                    1.0f
                )
            ) *
            brushRadius;

        glm::vec3 position =
            center;

        position.x +=
            std::cos(angle) *
            radius;

        position.z +=
            std::sin(angle) *
            radius;

        int finalBrushType =
            brushType;

        if (brushType == 4)
        {
            finalBrushType =
                std::rand() %
                4;
        }

        float scaleValue =
            WorldPainterRandomFloat(
                minScale,
                maxScale
            );

        if (finalBrushType == 0)
        {
            SpawnWorldPaintedModel(
                scene,
                selectedObject,
                shader,
                pineTreeModel,
                "Painted Pine Tree",
                "Assets/Models/Environment/NaturePack/PineTree_1.obj",
                position,
                scaleValue,
                false,
                randomRotation
            );
        }
        else if (finalBrushType == 1)
        {
            SpawnWorldPaintedModel(
                scene,
                selectedObject,
                shader,
                rockModel,
                "Painted Rock",
                "Assets/Models/Environment/NaturePack/Rock_1.obj",
                position,
                scaleValue,
                true,
                randomRotation
            );
        }
        else if (finalBrushType == 2)
        {
            SpawnWorldPaintedModel(
                scene,
                selectedObject,
                shader,
                grassModel,
                "Painted Grass",
                "Assets/Models/Environment/NaturePack/Grass.obj",
                position,
                scaleValue,
                false,
                randomRotation
            );
        }
        else if (finalBrushType == 3)
        {
            SpawnWorldPaintedModel(
                scene,
                selectedObject,
                shader,
                bushModel,
                "Painted Bush",
                "Assets/Models/Environment/NaturePack/Bush_1.obj",
                position,
                scaleValue,
                false,
                randomRotation
            );
        }
    }
}
void EditorUI::DrawAssetBrowser(
    Scene& scene,
    SceneObject*& selectedObject,
    Mesh* cubeMesh,
    Shader* shader,
    Material* cubeMaterial,
    Camera& camera,

    Model* woodenHouseModel,
    Model* newHouseModel,

    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel,

    std::function<void(bool)> spawnHouseCallback,
    std::function<void(bool)> buildCampCallback,
    std::function<void()> buildForestCallback
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            leftX,
            bottomY
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            assetBrowserWidth,
            bottomHeight
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Asset Browser");
    auto GetSpawnPosition =
        [&](float distance, float yPosition)
        {
            glm::vec3 forward =
                glm::vec3(
                    camera.Front.x,
                    0.0f,
                    camera.Front.z
                );

            if (glm::length(forward) < 0.001f)
            {
                forward =
                    glm::vec3(
                        0.0f,
                        0.0f,
                        -1.0f
                    );
            }

            forward =
                glm::normalize(
                    forward
                );

            glm::vec3 spawnPosition =
                camera.Position +
                forward * distance;

            spawnPosition.y =
                yPosition;

            return spawnPosition;
        };
    auto SpawnModelObject =
        [&](const std::string& objectName,
            Model* model,
            glm::vec3 scale,
            bool collider,
            const std::string& modelPath = "",
            const std::string& modelDirectory = "")
        {
            if (model == nullptr)
                return;

            SceneObject* object =
                new SceneObject(
                    model,
                    shader
                );

            object->name =
                objectName;

            glm::vec3 spawnPosition =
                GetSpawnPosition(
                    6.0f,
                    0.0f
                );

            object->transform.position =
                SnapEditorPositionToTerrain(
                    spawnPosition,
                    0.20f
                );

            object->transform.scale =
                scale;
            SetEditorSaveMetadata(
                object,
                "Model",
                "None",
                modelPath,
                modelDirectory
            );

            object->assetId =
                objectName;

            object->assetType =
                AssetType::Prop;

            object->spawnSource =
                SpawnSource::Manual;

            object->persistent =
                true;

            object->showInHierarchy =
                true;
            object->isCollider =
                collider;

            object->boundingRadius =
                50.0f;

            object->colliderRadius =
                glm::max(
                    scale.x,
                    scale.z
                ) * 0.8f;

            scene.AddObject(
                object
            );

            selectedObject =
                object;
        };

    auto SpawnCubeObject =
        [&](const std::string& objectName,
            glm::vec3 scale,
            glm::vec3 positionOffset,
            bool collider,
            glm::vec3 tint,
            glm::vec3 ambient,
            glm::vec3 diffuse)
        {
            Material* objectMaterial =
                new Material(
                    nullptr
                );

            objectMaterial->tint =
                tint;

            objectMaterial->ambient =
                ambient;

            objectMaterial->diffuse =
                diffuse;

            objectMaterial->specular =
                glm::vec3(
                    0.04f,
                    0.04f,
                    0.04f
                );

            objectMaterial->shininess =
                6.0f;

            SceneObject* object =
                new SceneObject(
                    cubeMesh,
                    shader,
                    objectMaterial
                );

            object->name =
                objectName;

            glm::vec3 spawnPosition =
                GetSpawnPosition(
                    6.0f,
                    0.0f
                );

            spawnPosition.x +=
                positionOffset.x;

            spawnPosition.z +=
                positionOffset.z;

            spawnPosition.y =
                GetTerrainHeight(
                    spawnPosition.x,
                    spawnPosition.z
                ) +
                positionOffset.y;

            object->transform.position =
                spawnPosition;

            object->transform.scale =
                scale;

            object->isCollider =
                collider;

            object->boundingRadius =
                50.0f;

            object->colliderRadius =
                glm::max(
                    scale.x,
                    scale.z
                ) * 0.8f;

            scene.AddObject(
                object
            );

            selectedObject =
                object;
        };

    if (ImGui::BeginTabBar("AssetBrowserTabs"))
    {
        if (ImGui::BeginTabItem("World Painter"))
        {
            static int brushType =
                4;

            static float brushRadius =
                14.0f;

            static int brushDensity =
                18;

            static float paintDistance =
                18.0f;

            static float minScale =
                0.75f;

            static float maxScale =
                1.75f;

            static bool randomRotation =
                true;

            static bool showBrushPreview =
                true;

            const char* brushNames[] =
            {
                "Pine Trees",
                "Rocks",
                "Grass",
                "Bushes",
                "Mixed Forest"
            };

            ImGui::Text(
                "Terrain Object Painting Tool"
            );

            ImGui::Separator();

            ImGui::Combo(
                "Brush Type",
                &brushType,
                brushNames,
                IM_ARRAYSIZE(
                    brushNames
                )
            );

            ImGui::SliderFloat(
                "Brush Radius",
                &brushRadius,
                4.0f,
                40.0f
            );

            ImGui::SliderInt(
                "Density",
                &brushDensity,
                1,
                80
            );

            ImGui::SliderFloat(
                "Paint Distance",
                &paintDistance,
                6.0f,
                70.0f
            );

            ImGui::SliderFloat(
                "Min Scale",
                &minScale,
                0.25f,
                3.0f
            );

            ImGui::SliderFloat(
                "Max Scale",
                &maxScale,
                0.25f,
                4.0f
            );

            if (maxScale < minScale)
            {
                maxScale =
                    minScale;
            }

            ImGui::Checkbox(
                "Random Rotation",
                &randomRotation
            );

            ImGui::Checkbox(
                "Show Brush Preview",
                &showBrushPreview
            );

            UpdateWorldPainterPreview(
                scene,
                shader,
                camera,
                brushRadius,
                paintDistance,
                showBrushPreview
            );

            ImGui::Separator();

            if (ImGui::Button("Paint Brush"))
            {
                PaintWorldBrush(
                    scene,
                    selectedObject,
                    shader,
                    camera,
                    brushType,
                    brushRadius,
                    brushDensity,
                    paintDistance,
                    minScale,
                    maxScale,
                    randomRotation,
                    pineTreeModel,
                    rockModel,
                    grassModel,
                    bushModel
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Small Forest Patch"))
            {
                PaintWorldBrush(
                    scene,
                    selectedObject,
                    shader,
                    camera,
                    4,
                    10.0f,
                    18,
                    paintDistance,
                    0.65f,
                    1.55f,
                    true,
                    pineTreeModel,
                    rockModel,
                    grassModel,
                    bushModel
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Dense Grass Patch"))
            {
                PaintWorldBrush(
                    scene,
                    selectedObject,
                    shader,
                    camera,
                    2,
                    16.0f,
                    50,
                    paintDistance,
                    0.35f,
                    0.95f,
                    true,
                    pineTreeModel,
                    rockModel,
                    grassModel,
                    bushModel
                );
            }

            ImGui::TextWrapped(
                "Move the editor camera, adjust Paint Distance, then press Paint Brush. The cyan ring shows where objects will be painted."
            );

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Nature"))
        {
            ImGui::Text("Trees");

            if (ImGui::Button("Pine Tree"))
            {
                SpawnModelObject(
                    "Pine Tree",
                    pineTreeModel,
                    glm::vec3(1.4f),
                    false,
                    "Assets/Models/Environment/NaturePack/PineTree_1.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Common Tree"))
            {
                SpawnModelObject(
                    "Common Tree",
                    commonTreeModel,
                    glm::vec3(1.4f),
                    false,
                    "Assets/Models/Environment/NaturePack/CommonTree_1.obj",
                    ""
                );
            }
            ImGui::SameLine();

            if (ImGui::Button("Small Pine"))
            {
                SpawnModelObject(
                    "Small Pine",
                    pineTreeModel,
                    glm::vec3(0.9f),
                    false,
                    "Assets/Models/Environment/NaturePack/PineTree_1.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Large Pine"))
            {
                SpawnModelObject(
                    "Large Pine",
                    pineTreeModel,
                    glm::vec3(2.2f),
                    true,
                    "Assets/Models/Environment/NaturePack/PineTree_1.obj",
                    ""
                );
            }

            ImGui::Separator();

            ImGui::Text("Rocks / Plants");

            if (ImGui::Button("Rock"))
            {
                SpawnModelObject(
                    "Rock",
                    rockModel,
                    glm::vec3(1.3f),
                    true,
                    "Assets/Models/Environment/NaturePack/Rock_1.obj",
                    ""
                );
            }

            ImGui::SameLine();
            ImGui::SameLine();

            if (ImGui::Button("Bush"))
            {
                SpawnModelObject(
                    "Bush",
                    bushModel,
                    glm::vec3(1.0f),
                    false,
                    "Assets/Models/Environment/NaturePack/Bush_1.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Grass"))
            {
                SpawnModelObject(
                    "Grass",
                    grassModel,
                    glm::vec3(0.5f),
                    false,
                    "Assets/Models/Environment/NaturePack/Grass.obj",
                    ""
                );
            }
            ImGui::SameLine();

            if (ImGui::Button("Small Rock"))
            {
                SpawnModelObject(
                    "Small Rock",
                    rockModel,
                    glm::vec3(0.7f),
                    true,
                    "Assets/Models/Environment/NaturePack/Rock_1.obj",
                    ""
                );
            }

            if (ImGui::Button("Large Rock"))
            {
                SpawnModelObject(
                    "Large Rock",
                    rockModel,
                    glm::vec3(2.0f),
                    true,
                    "Assets/Models/Environment/NaturePack/Rock_1.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Big Bush"))
            {
                SpawnModelObject(
                    "Big Bush",
                    bushModel,
                    glm::vec3(1.6f),
                    false,
                    "Assets/Models/Environment/NaturePack/Bush_1.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Tall Grass"))
            {
                SpawnModelObject(
                    "Tall Grass",
                    grassModel,
                    glm::vec3(0.9f),
                    false,
                    "Assets/Models/Environment/NaturePack/Grass.obj",
                    ""
                );
            }
            ImGui::Separator();

            ImGui::Text("Forest Props");

            if (ImGui::Button("Wood Log"))
            {
                SpawnModelObject(
                    "Wood Log",
                    woodLogModel,
                    glm::vec3(1.0f),
                    true,
                    "Assets/Models/Environment/NaturePack/WoodLog.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Tree Stump"))
            {
                SpawnModelObject(
                    "Tree Stump",
                    treeStumpModel,
                    glm::vec3(1.0f),
                    true,
                    "Assets/Models/Environment/NaturePack/TreeStump.obj",
                    ""
                );
            }
            ImGui::SameLine();

            if (ImGui::Button("Large Log"))
            {
                SpawnModelObject(
                    "Large Log",
                    woodLogModel,
                    glm::vec3(1.7f),
                    true,
                    "Assets/Models/Environment/NaturePack/WoodLog.obj",
                    ""
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Large Stump"))
            {
                SpawnModelObject(
                    "Large Stump",
                    treeStumpModel,
                    glm::vec3(1.6f),
                    true,
                    "Assets/Models/Environment/NaturePack/TreeStump.obj",
                    ""
                );
            }
            ImGui::EndTabItem();
        }
       

        if (ImGui::BeginTabItem("Structures"))
        {
            ImGui::Text("Houses");

            if (ImGui::Button("House 1"))
            {
                spawnHouseCallback(
                    false
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("House 2"))
            {
                spawnHouseCallback(
                    true
                );
            }

            ImGui::Separator();
            ImGui::Text("Camp Presets");

            if (ImGui::Button("Build Camp 1"))
            {
                buildCampCallback(
                    false
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Build Camp 2"))
            {
                buildCampCallback(
                    true
                );
            }

            ImGui::Separator();

            ImGui::Text("Fire Props");

            if (ImGui::Button("Campfire"))
            {
                SpawnCampfire(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            ImGui::Text("World Zones");

            if (ImGui::Button("Forest Zone"))
            {
                buildForestCallback();
            }

            ImGui::Separator();

            ImGui::Text("Building Pieces");

            if (ImGui::Button("Wall"))
            {
                SpawnCubeObject(
                    "Stone Wall",
                    glm::vec3(
                        4.0f,
                        2.5f,
                        0.25f
                    ),
                    glm::vec3(
                        0.0f,
                        1.25f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.42f,
                        0.40f,
                        0.36f
                    ),
                    glm::vec3(
                        0.35f,
                        0.35f,
                        0.35f
                    ),
                    glm::vec3(
                        0.75f,
                        0.72f,
                        0.66f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Fence Segment"))
            {
                SpawnCubeObject(
                    "Wood Fence Segment",
                    glm::vec3(
                        3.0f,
                        1.0f,
                        0.18f
                    ),
                    glm::vec3(
                        0.0f,
                        0.5f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.45f,
                        0.28f,
                        0.13f
                    ),
                    glm::vec3(
                        0.32f,
                        0.25f,
                        0.18f
                    ),
                    glm::vec3(
                        0.75f,
                        0.50f,
                        0.25f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Fence Post"))
            {
                SpawnCubeObject(
                    "Wood Fence Post",
                    glm::vec3(
                        0.3f,
                        1.3f,
                        0.3f
                    ),
                    glm::vec3(
                        0.0f,
                        0.65f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.34f,
                        0.20f,
                        0.10f
                    ),
                    glm::vec3(
                        0.28f,
                        0.20f,
                        0.14f
                    ),
                    glm::vec3(
                        0.65f,
                        0.42f,
                        0.22f
                    )
                );
            }
            ImGui::Separator();

            ImGui::Text("Advanced Building Pieces");

            if (ImGui::Button("Floor Tile"))
            {
                SpawnCubeObject(
                    "Floor Tile",
                    glm::vec3(
                        4.0f,
                        0.12f,
                        4.0f
                    ),
                    glm::vec3(
                        0.0f,
                        0.06f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.45f,
                        0.38f,
                        0.28f
                    ),
                    glm::vec3(
                        0.28f,
                        0.23f,
                        0.18f
                    ),
                    glm::vec3(
                        0.70f,
                        0.58f,
                        0.42f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Ceiling Tile"))
            {
                SpawnCubeObject(
                    "Ceiling Tile",
                    glm::vec3(
                        4.0f,
                        0.12f,
                        4.0f
                    ),
                    glm::vec3(
                        0.0f,
                        3.0f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.40f,
                        0.36f,
                        0.30f
                    ),
                    glm::vec3(
                        0.24f,
                        0.22f,
                        0.18f
                    ),
                    glm::vec3(
                        0.62f,
                        0.56f,
                        0.46f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Pillar"))
            {
                SpawnCubeObject(
                    "Stone Pillar",
                    glm::vec3(
                        0.55f,
                        3.0f,
                        0.55f
                    ),
                    glm::vec3(
                        0.0f,
                        1.5f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.42f,
                        0.41f,
                        0.38f
                    ),
                    glm::vec3(
                        0.28f,
                        0.28f,
                        0.26f
                    ),
                    glm::vec3(
                        0.70f,
                        0.68f,
                        0.62f
                    )
                );
            }

            if (ImGui::Button("Ramp"))
            {
                SpawnCubeObject(
                    "Ramp",
                    glm::vec3(
                        3.0f,
                        0.25f,
                        4.0f
                    ),
                    glm::vec3(
                        0.0f,
                        0.35f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.38f,
                        0.30f,
                        0.20f
                    ),
                    glm::vec3(
                        0.25f,
                        0.20f,
                        0.14f
                    ),
                    glm::vec3(
                        0.65f,
                        0.48f,
                        0.30f
                    )
                );

                if (selectedObject != nullptr)
                {
                    selectedObject->transform.rotation.x =
                        18.0f;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Door Frame"))
            {
                SpawnCubeObject(
                    "Door Frame Top",
                    glm::vec3(
                        2.2f,
                        0.30f,
                        0.35f
                    ),
                    glm::vec3(
                        0.0f,
                        2.45f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.38f,
                        0.22f,
                        0.10f
                    ),
                    glm::vec3(
                        0.24f,
                        0.15f,
                        0.08f
                    ),
                    glm::vec3(
                        0.65f,
                        0.40f,
                        0.20f
                    )
                );

                SpawnCubeObject(
                    "Door Frame Left",
                    glm::vec3(
                        0.30f,
                        2.4f,
                        0.35f
                    ),
                    glm::vec3(
                        -1.1f,
                        1.2f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.38f,
                        0.22f,
                        0.10f
                    ),
                    glm::vec3(
                        0.24f,
                        0.15f,
                        0.08f
                    ),
                    glm::vec3(
                        0.65f,
                        0.40f,
                        0.20f
                    )
                );

                SpawnCubeObject(
                    "Door Frame Right",
                    glm::vec3(
                        0.30f,
                        2.4f,
                        0.35f
                    ),
                    glm::vec3(
                        1.1f,
                        1.2f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.38f,
                        0.22f,
                        0.10f
                    ),
                    glm::vec3(
                        0.24f,
                        0.15f,
                        0.08f
                    ),
                    glm::vec3(
                        0.65f,
                        0.40f,
                        0.20f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Window Frame"))
            {
                SpawnCubeObject(
                    "Window Frame Top",
                    glm::vec3(
                        1.8f,
                        0.20f,
                        0.25f
                    ),
                    glm::vec3(
                        0.0f,
                        2.0f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.36f,
                        0.22f,
                        0.12f
                    ),
                    glm::vec3(
                        0.23f,
                        0.15f,
                        0.09f
                    ),
                    glm::vec3(
                        0.62f,
                        0.42f,
                        0.24f
                    )
                );

                SpawnCubeObject(
                    "Window Frame Bottom",
                    glm::vec3(
                        1.8f,
                        0.20f,
                        0.25f
                    ),
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.36f,
                        0.22f,
                        0.12f
                    ),
                    glm::vec3(
                        0.23f,
                        0.15f,
                        0.09f
                    ),
                    glm::vec3(
                        0.62f,
                        0.42f,
                        0.24f
                    )
                );

                SpawnCubeObject(
                    "Window Frame Left",
                    glm::vec3(
                        0.20f,
                        1.2f,
                        0.25f
                    ),
                    glm::vec3(
                        -0.9f,
                        1.5f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.36f,
                        0.22f,
                        0.12f
                    ),
                    glm::vec3(
                        0.23f,
                        0.15f,
                        0.09f
                    ),
                    glm::vec3(
                        0.62f,
                        0.42f,
                        0.24f
                    )
                );

                SpawnCubeObject(
                    "Window Frame Right",
                    glm::vec3(
                        0.20f,
                        1.2f,
                        0.25f
                    ),
                    glm::vec3(
                        0.9f,
                        1.5f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.36f,
                        0.22f,
                        0.12f
                    ),
                    glm::vec3(
                        0.23f,
                        0.15f,
                        0.09f
                    ),
                    glm::vec3(
                        0.62f,
                        0.42f,
                        0.24f
                    )
                );
            }
            ImGui::Separator();

            ImGui::Text("Shape Pieces");

            if (ImGui::Button("Ball"))
            {
                SpawnProceduralPrimitive(
                    scene,
                    selectedObject,
                    camera,
                    GetProceduralPrimitiveMesh(
                        "Sphere"
                    ),
                    shader,
                    "Primitive Ball",
                    glm::vec3(
                        1.0f
                    ),
                    glm::vec3(
                        0.75f,
                        0.75f,
                        0.78f
                    ),
                    0.5f,
                    1.2f
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Cylinder"))
            {
                SpawnProceduralPrimitive(
                    scene,
                    selectedObject,
                    camera,
                    GetProceduralPrimitiveMesh(
                        "Cylinder"
                    ),
                    shader,
                    "Primitive Cylinder",
                    glm::vec3(
                        1.0f
                    ),
                    glm::vec3(
                        0.55f,
                        0.55f,
                        0.52f
                    ),
                    0.5f,
                    1.2f
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Cone"))
            {
                SpawnProceduralPrimitive(
                    scene,
                    selectedObject,
                    camera,
                    GetProceduralPrimitiveMesh(
                        "Cone"
                    ),
                    shader,
                    "Primitive Cone",
                    glm::vec3(
                        1.0f
                    ),
                    glm::vec3(
                        0.60f,
                        0.45f,
                        0.28f
                    ),
                    0.5f,
                    1.2f
                );
            }
            ImGui::Separator();
            if (ImGui::Button("Stairs"))
            {
                SpawnStairsObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Arch"))
            {
                SpawnArchObject(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }
            if (ImGui::Button("Ring"))
            {
                SpawnRingObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Pipe"))
            {
                SpawnPipeObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }
            if (ImGui::Button("Capsule"))
            {
                SpawnCapsuleObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Roof Wedge"))
            {
                SpawnRoofWedgeObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }
            ImGui::Text("Outdoor Pieces");

            if (ImGui::Button("Path Tile"))
            {
                SpawnCubeObject(
                    "Dirt Path Tile",
                    glm::vec3(
                        3.0f,
                        0.05f,
                        3.0f
                    ),
                    glm::vec3(
                        0.0f,
                        0.04f,
                        0.0f
                    ),
                    false,
                    glm::vec3(
                        0.42f,
                        0.31f,
                        0.18f
                    ),
                    glm::vec3(
                        0.32f,
                        0.25f,
                        0.18f
                    ),
                    glm::vec3(
                        0.70f,
                        0.52f,
                        0.30f
                    )
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Wood Platform"))
            {
                SpawnCubeObject(
                    "Wood Platform",
                    glm::vec3(
                        4.0f,
                        0.2f,
                        4.0f
                    ),
                    glm::vec3(
                        0.0f,
                        0.12f,
                        0.0f
                    ),
                    true,
                    glm::vec3(
                        0.48f,
                        0.30f,
                        0.14f
                    ),
                    glm::vec3(
                        0.32f,
                        0.24f,
                        0.16f
                    ),
                    glm::vec3(
                        0.78f,
                        0.52f,
                        0.28f
                    )
                );
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lights"))
        {
            ImGui::Text("Torch lights are created from the toolbar.");
            ImGui::Text("Use Add Light to spawn a torch object with a point light.");
            ImGui::Separator();
            ImGui::Text("Current safe light limit: 5 active torch lights.");

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Gameplay"))
        {
            ImGui::Text("Gameplay Presets");

            if (ImGui::Button("Build Coin Path"))
            {
                BuildCoinPathPreset(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Build Monster Encounter"))
            {
                BuildMonsterEncounterPreset(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            if (ImGui::Button("Build Music Rescue Setup"))
            {
                BuildMusicRescuePreset(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Build Full Demo Setup"))
            {
                BuildDemoScene(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            ImGui::TextDisabled(
                "Preset tools create ready-to-test gameplay setups in front of the camera."
            );

            ImGui::Separator();

            ImGui::Text("Single Gameplay Objects");

            if (ImGui::Button("Coin"))
            {
                SpawnCoinObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Coin x10"))
            {
                for (int i = 0; i < 10; i++)
                {
                    SceneObject* coin =
                        SpawnCoinObject(
                            scene,
                            selectedObject,
                            camera,
                            shader
                        );

                    if (coin != nullptr)
                    {
                        coin->transform.position.x +=
                            (float)(i % 5) * 1.8f;

                        coin->transform.position.z +=
                            (float)(i / 5) * 1.8f;

                        PlaceCoinOnTerrain(
                            coin,
                            0.18f
                        );

                        coin->name =
                            "Coin " +
                            std::to_string(
                                i + 1
                            );
                    }
                }
            }
            ImGui::SameLine();

            if (ImGui::Button("Random placed Coins"))
            {
                glm::vec3 forward =
                    glm::vec3(
                        camera.Front.x,
                        0.0f,
                        camera.Front.z
                    );

                if (glm::length(forward) < 0.001f)
                {
                    forward =
                        glm::vec3(
                            0.0f,
                            0.0f,
                            -1.0f
                        );
                }

                forward =
                    glm::normalize(
                        forward
                    );

                glm::vec3 center =
                    camera.Position +
                    forward * 10.0f;

                for (int i = 0; i < 15; i++)
                {
                    SceneObject* coin =
                        SpawnCoinObject(
                            scene,
                            selectedObject,
                            camera,
                            shader
                        );

                    if (coin != nullptr)
                    {
                        float randomX =
                            ((float)(rand() % 1000) / 1000.0f - 0.5f) * 18.0f;

                        float randomZ =
                            ((float)(rand() % 1000) / 1000.0f - 0.5f) * 18.0f;

                        coin->transform.position.x =
                            center.x + randomX;

                        coin->transform.position.z =
                            center.z + randomZ;

                        PlaceCoinOnTerrain(
                            coin,
                            0.18f
                        );

                        coin->name =
                            "Random Coin " +
                            std::to_string(
                                i + 1
                            );
                    }
                }
            }
            ImGui::Separator();

            ImGui::Text("Trigger Event Objects");

            if (ImGui::Button("Trigger Zone"))
            {
                SpawnTriggerZoneObject(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Monster Spawn"))
            {
                SpawnMonsterSpawnObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }
            ImGui::Separator();

            ImGui::Text("Music Rescue Event");
            ImGui::Separator();

            ImGui::Text("Legacy Demo Tool");

            if (ImGui::Button("Build Basic Demo Scene"))
            {
                BuildDemoScene(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }
            if (ImGui::Button("Music Gate"))
            {
                SpawnMusicGateObject(
                    scene,
                    selectedObject,
                    camera,
                    cubeMesh,
                    shader
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Music NPC"))
            {
                SpawnMusicNpcObject(
                    scene,
                    selectedObject,
                    camera,
                    shader
                );
            }
            ImGui::Separator();

            ImGui::Text("Gameplay tools:");
            ImGui::BulletText("Coin creates a collectible object.");
            ImGui::BulletText("Use Duplicate to place more coins.");
            ImGui::BulletText("Save/Load keeps coins using V2 metadata.");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
// ================= SELECTION TOOLS V1 =================

static bool IsSelectionToolIgnoredObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return true;

    if (object->name == "Procedural Terrain")
        return true;

    if (object->name == "Ground")
        return true;

    if (object->name == "World Painter Preview")
        return true;

    if (object->name.find("Generated") != std::string::npos)
        return true;

    return false;
}

static SceneObject* FindEditorPlayerObject(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (object->name == "Player")
            return object;

        if (object->assetType == AssetType::Player)
            return object;
    }

    return nullptr;
}

static SceneObject* FindNearestEditorObjectToCamera(
    Scene& scene,
    Camera& camera
)
{
    SceneObject* nearestObject =
        nullptr;

    float nearestDistance =
        99999999.0f;

    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (!object->visible)
            continue;

        if (IsSelectionToolIgnoredObject(object))
            continue;

        glm::vec3 delta =
            object->transform.position -
            camera.Position;

        float distance =
            glm::length(
                delta
            );

        if (distance < nearestDistance)
        {
            nearestDistance =
                distance;

            nearestObject =
                object;
        }
    }

    return nearestObject;
}

static void FrameSelectedEditorObject(
    SceneObject* selectedObject,
    Camera& camera
)
{
    if (selectedObject == nullptr)
        return;

    glm::vec3 forward =
        camera.Front;

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    float objectSize =
        glm::length(
            selectedObject->transform.scale
        );

    float distance =
        glm::clamp(
            objectSize * 3.0f,
            5.0f,
            22.0f
        );

    camera.Position =
        selectedObject->transform.position -
        forward * distance +
        glm::vec3(
            0.0f,
            distance * 0.35f,
            0.0f
        );

    std::cout
        << "Camera framed selected object: "
        << selectedObject->name
        << std::endl;
}

static void MoveSelectedEditorObjectInFrontOfCamera(
    SceneObject* selectedObject,
    Camera& camera
)
{
    if (selectedObject == nullptr)
        return;

    glm::vec3 forward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(forward) < 0.001f)
    {
        forward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    forward =
        glm::normalize(
            forward
        );

    glm::vec3 newPosition =
        camera.Position +
        forward * 6.0f;

    float oldTerrainY =
        GetTerrainHeight(
            selectedObject->transform.position.x,
            selectedObject->transform.position.z
        );

    float heightOffset =
        selectedObject->transform.position.y -
        oldTerrainY;

    newPosition.y =
        GetTerrainHeight(
            newPosition.x,
            newPosition.z
        ) +
        heightOffset;

    selectedObject->transform.position =
        newPosition;

    std::cout
        << "Moved selected object in front of camera."
        << std::endl;
}

static void SnapSelectedEditorObjectToGround(
    SceneObject* selectedObject
)
{
    if (selectedObject == nullptr)
        return;

    selectedObject->transform.position =
        SnapEditorPositionToTerrain(
            selectedObject->transform.position,
            0.10f
        );

    std::cout
        << "Snapped selected object to terrain."
        << std::endl;
}

static void DeleteSelectedEditorObjectSafe(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    if (selectedObject == nullptr)
        return;

    if (selectedObject->name == "Player")
    {
        std::cout
            << "Delete skipped: Player object should not be deleted."
            << std::endl;

        return;
    }

    if (selectedObject->name == "Procedural Terrain")
    {
        std::cout
            << "Delete skipped: Terrain object should not be deleted."
            << std::endl;

        return;
    }

    if (selectedObject->parent != nullptr)
    {
        auto& siblings =
            selectedObject->parent->children;

        siblings.erase(
            std::remove(
                siblings.begin(),
                siblings.end(),
                selectedObject
            ),
            siblings.end()
        );
    }

    for (SceneObject* child : selectedObject->children)
    {
        if (child != nullptr)
        {
            child->parent =
                nullptr;
        }
    }

    selectedObject->children.clear();

    if (selectedObject->attachedLight != nullptr)
    {
        Light* attachedLight =
            selectedObject->attachedLight;

        scene.lights.erase(
            std::remove(
                scene.lights.begin(),
                scene.lights.end(),
                attachedLight
            ),
            scene.lights.end()
        );

        delete attachedLight;

        selectedObject->attachedLight =
            nullptr;
    }

    for (auto it = scene.objects.begin();
        it != scene.objects.end();
        ++it)
    {
        if (*it == selectedObject)
        {
            std::cout
                << "Deleted object: "
                << selectedObject->name
                << std::endl;

            delete* it;

            scene.objects.erase(
                it
            );

            selectedObject =
                nullptr;

            break;
        }
    }
}
void EditorUI::DrawToolbar(
    Scene& scene,
    SceneObject*& selectedObject,
    Light*& selectedLight,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    Camera& camera,
    Model* torchModel,
    int& lightCounter,
    AppMode& appMode,
    std::function<void()> saveEditorCallback,
    std::function<void()> loadEditorCallback
)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(1600, 45), ImGuiCond_FirstUseEver);
    ImGui::Begin("Toolbar");
    if (appMode == AppMode::Editor)
    {
        if (ImGui::Button("Play##ToolbarPlay"))
        {
            for (SceneObject* object : scene.objects)
            {
                if (object == nullptr)
                    continue;

                if (object->editorGameplayType == "Coin")
                {
                    object->visible =
                        true;
                }
            }

            appMode =
                AppMode::Play;

            selectedObject =
                nullptr;
        }
    }
    else
    {
        if (ImGui::Button("Stop##ToolbarStop"))
        {
            appMode = AppMode::Editor;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        saveEditorCallback();
    }

    ImGui::SameLine();

    if (ImGui::Button("Load"))
    {
        loadEditorCallback();
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Cube##Toolbar"))
    {
        SceneObject* obj =
            new SceneObject(
                cubeMesh,
                shader,
                material
            );

        obj->name = "New Cube";

        scene.AddObject(obj);

        selectedObject = obj;
    }
    ImGui::SameLine();

    if (ImGui::Button("Duplicate##Toolbar"))
    {
        DuplicateSelectedEditorObject(
            scene,
            selectedObject
        );
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Light"))
    {
        glm::vec3 forward =
            glm::vec3(
                camera.Front.x,
                0.0f,
                camera.Front.z
            );

        if (glm::length(forward) < 0.001f)
        {
            forward =
                glm::vec3(
                    0.0f,
                    0.0f,
                    -1.0f
                );
        }

        forward =
            glm::normalize(
                forward
            );

        glm::vec3 spawnPosition =
            camera.Position +
            forward * 6.0f;

        spawnPosition.y =
            0.05f;

        Light* torchLight =
            new Light();

        torchLight->name =
            "Torch Light " +
            std::to_string(lightCounter++);

        torchLight->type =
            LightType::Point;

        torchLight->color =
            glm::vec3(
                3.5f,
                1.8f,
                0.55f
            );

        torchLight->position =
            spawnPosition +
            glm::vec3(
                0.0f,
                1.8f,
                0.0f
            );

        scene.AddLight(
            torchLight
        );

        SceneObject* torchObject =
            new SceneObject(
                torchModel,
                shader
            );

        torchObject->name =
            torchLight->name + " Object";
        spawnPosition.y =
            GetTerrainHeight(
                spawnPosition.x,
                spawnPosition.z
            ) + 0.10f;
        torchObject->transform.position =
            spawnPosition;

        torchObject->transform.scale =
            glm::vec3(
                1.0f
            );

        torchObject->isCollider =
            false;

        torchObject->boundingRadius =
            20.0f;

        torchObject->attachedLight =
            torchLight;

        torchObject->attachedLightOffset =
            glm::vec3(
                0.0f,
                1.8f,
                0.0f
            );

        scene.AddObject(
            torchObject
        );

        selectedLight =
            torchLight;

        selectedObject =
            torchObject;
    }

    ImGui::SameLine();

    if (ImGui::Button("Delete##Toolbar"))
    {
        DeleteSelectedEditorObjectSafe(
            scene,
            selectedObject
        );
    }
    ImGui::SameLine();
    ImGui::SameLine();

    ImGui::Text("| Selection:");

    ImGui::SameLine();

    if (ImGui::Button("Select Player##Toolbar"))
    {
        selectedObject =
            FindEditorPlayerObject(
                scene
            );

        selectedLight =
            nullptr;
    }

    ImGui::SameLine();

    if (ImGui::Button("Nearest##Toolbar"))
    {
        selectedObject =
            FindNearestEditorObjectToCamera(
                scene,
                camera
            );

        selectedLight =
            nullptr;
    }

    ImGui::SameLine();

    if (ImGui::Button("Frame##Toolbar"))
    {
        FrameSelectedEditorObject(
            selectedObject,
            camera
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Move Front##Toolbar"))
    {
        MoveSelectedEditorObjectInFrontOfCamera(
            selectedObject,
            camera
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Snap Ground##Toolbar"))
    {
        SnapSelectedEditorObjectToGround(
            selectedObject
        );
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Selected Prefab"))
    {
        if (selectedObject != nullptr)
        {
            Prefab prefab;

            prefab.name =
                selectedObject->name;

            // ================= IDENTITY =================

            prefab.meshType =
                selectedObject->editorMeshType;

            prefab.modelPath =
                selectedObject->editorModelPath;

            prefab.modelDirectory =
                selectedObject->editorModelDirectory;

            prefab.texturePath =
                selectedObject->editorTexturePath;

            prefab.gameplayType =
                selectedObject->editorGameplayType;

            // ================= TRANSFORM =================

            prefab.position =
                selectedObject->transform.position;

            prefab.rotation =
                selectedObject->transform.rotation;

            prefab.scale =
                selectedObject->transform.scale;

            // ================= COLLISION =================

            prefab.isCollider =
                selectedObject->isCollider;

            prefab.colliderRadius =
                selectedObject->colliderRadius;

            PrefabManager::SavePrefab(
                prefab
            );

            std::cout
                << "Prefab saved: "
                << prefab.name
                << std::endl;
        }
    }
    ImGui::SameLine();

    ImGui::Text("|");

    ImGui::SameLine();
    if (appMode == AppMode::Play)
    {
        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "MODE: PLAY"
        );

        ImGui::SameLine();

        ImGui::Text(
            "Runtime gameplay active"
        );
    }
    else
    {
        ImGui::TextColored(
            ImVec4(
                0.3f,
                0.7f,
                1.0f,
                1.0f
            ),
            "MODE: EDITOR"
        );

        ImGui::SameLine();

        ImGui::Text(
            "Scene editing active"
        );
    }
    ImGui::End();
}
void EditorUI::DrawCrosshair()
{
    ImGuiIO& io =
        ImGui::GetIO();

    ImDrawList* draw =
        ImGui::GetForegroundDrawList();

    ImVec2 center(
        io.DisplaySize.x * 0.5f,
        io.DisplaySize.y * 0.5f
    );

    draw->AddLine(
        ImVec2(center.x - 8, center.y),
        ImVec2(center.x + 8, center.y),
        IM_COL32(255, 255, 255, 255),
        2.0f
    );

    draw->AddLine(
        ImVec2(center.x, center.y - 8),
        ImVec2(center.x, center.y + 8),
        IM_COL32(255, 255, 255, 255),
        2.0f
    );
}
void EditorUI::DrawStatistics(
    Scene& scene,
    Camera& camera,
    SceneObject* selectedObject,
    float deltaTime
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            centerX + 10.0f,
            topY + 10.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            230.0f,
            165.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin("Statistics");
    ImGui::Text(
        "Objects : %d",
        (int)scene.objects.size()
    );

    ImGui::Text(
        "Lights : %d",
        (int)scene.lights.size()
    );

    ImGui::Separator();

    ImGui::Text(
        "FPS : %.1f",
        1.0f / deltaTime
    );

    ImGui::Separator();

    ImGui::Text("Camera");

    ImGui::Text(
        "X : %.2f",
        camera.Position.x
    );

    ImGui::Text(
        "Y : %.2f",
        camera.Position.y
    );

    ImGui::Text(
        "Z : %.2f",
        camera.Position.z
    );

    /*
 ImGui::Separator();

 if (selectedObject)
 {
     ImGui::Text("Selected:");

     ImGui::Text(
         "%s",
         selectedObject->name.c_str()
     );
 }
 else
 {
     ImGui::Text("Selected: None");
 }
 */

    ImGui::End();
}