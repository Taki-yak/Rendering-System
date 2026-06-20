#include "EditorUI.h"
#include "EditorUI.h"
#include "Light.h"
#include <glm/gtc/type_ptr.hpp>
#include "SceneSerializer.h"
void DrawHierarchyNode(SceneObject* obj, SceneObject*& selectedObject)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    if (obj == selectedObject)
        flags |= ImGuiTreeNodeFlags_Selected;

    std::string displayName =
        obj->visible ?
        obj->name :
        obj->name + " (Hidden)";

    std::string id =
        displayName +
        "##" +
        std::to_string((size_t)obj);

    bool opened = ImGui::TreeNodeEx(
        id.c_str(),
        flags
    );


    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        selectedObject = obj;
    }
    if (opened)
    {
        for (SceneObject* child : obj->children)
        {
            DrawHierarchyNode(child, selectedObject);
        }

        ImGui::TreePop();
    }
}
void EditorUI::DrawLightInspector(
    Light* selectedLight
)
{
    ImGui::Begin("Light Inspector");

    if (selectedLight)
    {
        static char buffer[128];

        strcpy_s(
            buffer,
            selectedLight->name.c_str()
        );

        if (ImGui::InputText(
            "Name",
            buffer,
            IM_ARRAYSIZE(buffer)))
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
            "No light selected"
        );
    }

    ImGui::End();
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
    for (SceneObject* obj : scene.objects)
    {
        if (obj->parent == nullptr)
        {
            DrawHierarchyNode(obj, selectedObject);
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
        ImGui::BulletText(
            "%s",
            light->name.c_str()
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
        static char nameBuffer[128];

        strcpy_s(
            nameBuffer,
            selectedObject->name.c_str()
        );

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
    }
    else
    {
        ImGui::Text("No object selected");
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
void EditorUI::DrawToolbar(
    Scene& scene,
    SceneObject*& selectedObject,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    int& lightCounter
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

    ImGui::End();
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