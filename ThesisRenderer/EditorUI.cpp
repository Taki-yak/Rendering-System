#include "EditorUI.h"
#include "EditorUI.h"
#include "Light.h"
#include <glm/gtc/type_ptr.hpp>
#include "SceneSerializer.h"
#include <algorithm>
#include "PrefabManager.h"
#include "AssetDatabase.h"
#include <glm/glm.hpp>
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
void EditorUI::DrawHierarchy(

    Scene& scene,
    SceneObject*& selectedObject,
    Light*& selectedLight
)
{
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(
            10.0f,
            70.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            260.0f,
            360.0f
        ),
        ImGuiCond_Once
    );
    ImGui::Begin("Hierarchy");
    static char searchBuffer[128] = "";

    ImGui::InputText(
        "Search",
        searchBuffer,
        IM_ARRAYSIZE(searchBuffer)
    );

    ImGui::Separator();
    for (SceneObject* obj : scene.objects)
    {
        std::string objectName = obj->name;

        if (strlen(searchBuffer) > 0)
        {
            if (
                objectName.find(searchBuffer)
                == std::string::npos
                )
            {
                continue;
            }
        }

        if (obj->parent == nullptr)
        {
            DrawHierarchyNode(
                obj,
                selectedObject
            );
        }
    }

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

    for (auto& prefab :
        PrefabManager::prefabs)
    {
        if (ImGui::TreeNode("Models"))
        {
            for (auto& asset :
                AssetDatabase::assets)
            {
                ImGui::Selectable(
                    asset.name.c_str()
                );
            }

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

void EditorUI::DrawInspector(
    SceneObject* selectedObject
)
{
    ImGui::SetNextWindowPos(ImVec2(550, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(
            1240.0f,
            70.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            340.0f,
            280.0f
        ),
        ImGuiCond_Once
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
        ImGui::Checkbox(
            "Visible",
            &selectedObject->visible
        );
        ImGui::Checkbox(
            "Collider",
            &selectedObject->isCollider
        );
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

    ImGui::End();
}
void EditorUI::DrawLightInspector(
    Light* selectedLight
)
{

    ImGui::SetNextWindowPos(
        ImVec2(
            930.0f,
            70.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            300.0f,
            260.0f
        ),
        ImGuiCond_Once
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
    ImGui::SetNextWindowPos(ImVec2(260, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(280, 200), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(
            10.0f,
            440.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            260.0f,
            190.0f
        ),
        ImGuiCond_Once
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
    obj->transform.position = position;
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
void EditorUI::DrawAssetBrowser(
    Scene& scene,
    SceneObject*& selectedObject,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    Camera& camera,

    Model* woodenHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            10.0f,
            650.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            900.0f,
            230.0f
        ),
        ImGuiCond_Once
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
            bool collider)
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
                GetSpawnPosition(
                    6.0f,
                    0.05f
                );

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

    auto SpawnCubeObject =
        [&](const std::string& objectName,
            glm::vec3 scale,
            glm::vec3 positionOffset,
            bool collider)
        {
            SceneObject* object =
                new SceneObject(
                    cubeMesh,
                    shader,
                    material
                );

            object->name =
                objectName;

            object->transform.position =
                GetSpawnPosition(
                    6.0f,
                    0.0f
                ) +
                positionOffset;

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
        if (ImGui::BeginTabItem("Nature"))
        {
            ImGui::Text("Trees");

            if (ImGui::Button("Pine Tree"))
            {
                SpawnModelObject(
                    "Pine Tree",
                    pineTreeModel,
                    glm::vec3(
                        0.8f
                    ),
                    false
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Common Tree"))
            {
                SpawnModelObject(
                    "Common Tree",
                    commonTreeModel,
                    glm::vec3(
                        0.9f
                    ),
                    false
                );
            }

            ImGui::Separator();

            ImGui::Text("Rocks / Plants");

            if (ImGui::Button("Rock"))
            {
                SpawnModelObject(
                    "Rock",
                    rockModel,
                    glm::vec3(
                        0.8f
                    ),
                    true
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Bush"))
            {
                SpawnModelObject(
                    "Bush",
                    bushModel,
                    glm::vec3(
                        0.8f
                    ),
                    false
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Grass"))
            {
                SpawnModelObject(
                    "Grass",
                    grassModel,
                    glm::vec3(
                        0.7f
                    ),
                    false
                );
            }

            ImGui::Separator();

            ImGui::Text("Forest Props");

            if (ImGui::Button("Wood Log"))
            {
                SpawnModelObject(
                    "Wood Log",
                    woodLogModel,
                    glm::vec3(
                        1.0f
                    ),
                    true
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Tree Stump"))
            {
                SpawnModelObject(
                    "Tree Stump",
                    treeStumpModel,
                    glm::vec3(
                        0.9f
                    ),
                    true
                );
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Structures"))
        {
            ImGui::Text("Real Structure Models");

            if (ImGui::Button("Wooden House"))
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

                SceneObject* house =
                    new SceneObject(
                        woodenHouseModel,
                        shader
                    );

                house->name =
                    "Wooden House";

                house->transform.position =
                    camera.Position +
                    forward * 8.0f;

                house->transform.position.y =
                    0.0f;

                house->transform.scale =
                    glm::vec3(
                        1.0f
                    );

                house->isCollider =
                    true;

                house->boundingRadius =
                    100.0f;

                house->colliderRadius =
                    6.0f;

                scene.AddObject(
                    house
                );

                selectedObject =
                    house;
            }

            ImGui::Separator();
            ImGui::Text("Basic Building Pieces");

            if (ImGui::Button("Wall"))
            {
                SpawnCubeObject(
                    "Wall",
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
                    true
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Fence Segment"))
            {
                SpawnCubeObject(
                    "Fence Segment",
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
                    true
                );
            }

            ImGui::SameLine();

            if (ImGui::Button("Fence Post"))
            {
                SpawnCubeObject(
                    "Fence Post",
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
                    true
                );
            }

            ImGui::Separator();

            ImGui::Text("Camp Layout Pieces");
            if (ImGui::Button("Build Real Camp"))
            {
                BuildRealCamp(
                    scene,
                    selectedObject,
                    camera,
                    shader,
                    woodenHouseModel,
                    pineTreeModel,
                    commonTreeModel,
                    rockModel,
                    bushModel,
                    woodLogModel,
                    treeStumpModel
                );
            }
            if (ImGui::Button("Path Tile"))
            {
                SpawnCubeObject(
                    "Path Tile",
                    glm::vec3(
                        3.0f,
                        0.05f,
                        3.0f
                    ),
                    glm::vec3(
                        0.0f,
                        0.03f,
                        0.0f
                    ),
                    false
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
                        0.1f,
                        0.0f
                    ),
                    true
                );
            }

            ImGui::SameLine();

         

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
            ImGui::Text("Gameplay tools are in the Player Tools window.");
            ImGui::Text("Available tools:");
            ImGui::BulletText("Place Player In Front Of Camera");
            ImGui::BulletText("Set Spawn Here");
            ImGui::BulletText("Respawn Player");
            ImGui::BulletText("Select Player");

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
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
    AppMode& appMode
)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(1600, 45), ImGuiCond_Always);
    ImGui::Begin("Toolbar");
    if (appMode == AppMode::Editor)
    {
        if (ImGui::Button("Play##ToolbarPlay"))
        {
            appMode = AppMode::Play;
            selectedObject = nullptr;
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
        SceneSerializer::Save(
            scene,
            "scene.txt"
        );
    }

    ImGui::SameLine();

    if (ImGui::Button("Load"))
    {

        SceneSerializer::Load(
            scene,
            "scene.txt",
            cubeMesh,
            shader,
            material
        );
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
        if (selectedObject)
        {
            for (auto it = scene.objects.begin();
                it != scene.objects.end();
                ++it)
            {
                if (*it == selectedObject)
                {
                    delete* it;

                    scene.objects.erase(it);

                    selectedObject = nullptr;

                    break;
                }
            }
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Save Prefab"))
    {
        if (selectedObject)
        {
            Prefab prefab;

            prefab.name =
                selectedObject->name;

            prefab.position =
                selectedObject->transform.position;

            prefab.rotation =
                selectedObject->transform.rotation;

            prefab.scale =
                selectedObject->transform.scale;

            PrefabManager::SavePrefab(
                prefab
            );
        }
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
            280.0f,
            70.0f
        ),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            260.0f,
            160.0f
        ),
        ImGuiCond_Once
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