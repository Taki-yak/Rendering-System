#pragma once

#include "imgui.h"
#include "Scene.h"

class EditorUI
{
public:

    static void DrawHierarchy(
        Scene& scene,
        SceneObject*& selectedObject
    );

    static void DrawInspector(
        SceneObject* selectedObject
    );

    static void DrawDebug(
        float deltaTime,
        int totalObjects,
        int visibleObjects,
        int culledObjects,
        SceneObject* selectedObject
    );
    static void DrawToolbar(
        Scene& scene,
        SceneObject*& selectedObject,
        Mesh* cubeMesh,
        Shader* shader,
        Material* material
    );
};