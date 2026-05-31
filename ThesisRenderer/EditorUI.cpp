#include "EditorUI.h"

#include <glm/gtc/type_ptr.hpp>

void DrawHierarchyNode(SceneObject* obj, SceneObject*& selectedObject)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

    if (obj == selectedObject)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool opened = ImGui::TreeNodeEx(
        obj->name.c_str(),
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
void EditorUI::DrawHierarchy(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    ImGui::Begin("Hierarchy");
    for (SceneObject* obj : scene.objects)
    {
     
        if (obj->parent == nullptr)
        {
            DrawHierarchyNode(obj, selectedObject);
        }
    }

    ImGui::End();
}

void EditorUI::DrawInspector(
    SceneObject* selectedObject
)
{
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
    Material* material)
{
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

    if (ImGui::Button("Add Cube"))
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

    if (ImGui::Button("Duplicate"))
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

    if (ImGui::Button("Delete"))
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