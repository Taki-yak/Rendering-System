#include "EditorUI.h"
#include "EditorUI.h"
#include "Light.h"
#include <glm/gtc/type_ptr.hpp>
#include "SceneSerializer.h"
#include <algorithm>
#include "PrefabManager.h"
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
        ImGui::BulletText(
            "%s",
            prefab.name.c_str()
        );
    }
    ImGui::End();
}

void EditorUI::DrawInspector(
    SceneObject* selectedObject
)
{
    ImGui::SetNextWindowPos(ImVec2(550, 20), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_Once);
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
        ImGui::Separator();

        ImGui::Text("Material");
        Material* mat =
            selectedObject->material;
        ImGui::ColorEdit3(
            "Ambient",
            glm::value_ptr(
                mat->ambient
            )
        );

        ImGui::ColorEdit3(
            "Diffuse",
            glm::value_ptr(
                mat->diffuse
            )
        );

        ImGui::ColorEdit3(
            "Specular",
            glm::value_ptr(
                mat->specular
            )
        );
        ImGui::DragFloat(
            "Shininess",
            &mat->shininess,
            1.0f,
            1.0f,
            256.0f
        );
        ImGui::Checkbox(
            "Wireframe",
            &mat->wireframe
        );
        ImGui::ColorEdit3(
            "Tint",
            glm::value_ptr(
                mat->tint
            )
        );

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
void EditorUI::DrawAssetBrowser(
    Scene& scene,
    SceneObject*& selectedObject,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material
)
{
    ImGui::SetNextWindowPos(
        ImVec2(10, 520),
        ImGuiCond_Once
    );

    ImGui::SetNextWindowSize(
        ImVec2(530, 180),
        ImGuiCond_Once
    );

    ImGui::Begin("Asset Browser");

    if (ImGui::TreeNode("Textures"))
    {
        ImGui::Selectable("container.jpg");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Models"))
    {
        ImGui::Selectable("character-human.obj");
        ImGui::Selectable("character-a.obj");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Materials"))
    {
        ImGui::Selectable("Default Material");
        ImGui::TreePop();
    }

    ImGui::Separator();

    ImGui::Text("Environment Props");

    if (ImGui::Button("Spawn Crate"))
    {
        SceneObject* crate =
            new SceneObject(
                cubeMesh,
                shader,
                material
            );

        crate->name = "Crate";
        crate->isCollider = true;
        crate->transform.position =
            glm::vec3(
                0.0f,
                0.5f,
                -5.0f
            );

        crate->transform.scale =
            glm::vec3(
                1.0f,
                1.0f,
                1.0f
            );

        scene.AddObject(crate);

        selectedObject = crate;
    }

    ImGui::SameLine();

    if (ImGui::Button("Spawn Wall"))
    {
        SceneObject* wall =
            new SceneObject(
                cubeMesh,
                shader,
                material
            );

        wall->name = "Wall";
        wall->isCollider = true;
        wall->transform.position =
            glm::vec3(
                0.0f,
                1.5f,
                -8.0f
            );

        wall->transform.scale =
            glm::vec3(
                6.0f,
                3.0f,
                0.3f
            );

        scene.AddObject(wall);

        selectedObject = wall;
    }

    ImGui::SameLine();

    if (ImGui::Button("Spawn Pillar"))
    {
        SceneObject* pillar =
            new SceneObject(
                cubeMesh,
                shader,
                material
            );

        pillar->name = "Pillar";
        pillar->isCollider = true;
        pillar->transform.position =
            glm::vec3(
                3.0f,
                1.5f,
                -6.0f
            );

        pillar->transform.scale =
            glm::vec3(
                0.7f,
                3.0f,
                0.7f
            );

        scene.AddObject(pillar);

        selectedObject = pillar;
    }

    ImGui::End();
}void EditorUI::DrawToolbar(
    Scene& scene,
    SceneObject*& selectedObject,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    int& lightCounter,
    AppMode& appMode
)
{
    ImGui::SetNextWindowPos(
        ImVec2(0, 0),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(800, 40),
        ImGuiCond_Always
    );
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
        if (ImGui::Button("Stop"))
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
        Light* newLight = new Light();

        newLight->name =
            "Light_" +
            std::to_string(lightCounter++);

        newLight->position =
            glm::vec3(
                0.0f,
                3.0f,
                0.0f
            );

        scene.AddLight(newLight);
    }
    ImGui::SameLine();

    if (ImGui::Button("Duplicate##Toolbar"))
    {
        if (selectedObject)
        {
            SceneObject* copy =
                new SceneObject(
                    selectedObject->mesh,
                    selectedObject->shader,
                    selectedObject->material
                );

            copy->transform =
                selectedObject->transform;

            copy->transform.position.x += 1.0f;

            scene.AddObject(copy);

            selectedObject = copy;
        }
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