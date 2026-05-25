#include "EditorUI.h"

#include <glm/gtc/type_ptr.hpp>

void EditorUI::DrawHierarchy(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    ImGui::Begin("Hierarchy");

    for (SceneObject* obj : scene.objects)
    {
        bool isSelected =
            (selectedObject == obj);

        if (ImGui::Selectable(
            obj->name.c_str(),
            isSelected))
        {
            if (selectedObject != nullptr)
                selectedObject->isSelected = false;

            selectedObject = obj;
            selectedObject->isSelected = true;
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