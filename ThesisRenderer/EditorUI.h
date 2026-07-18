#pragma once

#include "imgui.h"
#include "Scene.h"
#include "Camera.h"
#include "AppMode.h"
#include "Light.h"
#include "Model.h"
#include "Camera.h"
class EditorUI
{
public:
    static void DrawAssetBrowser(
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
    );
    static void DrawCrosshair();
    static void DrawHierarchy(
        Scene& scene,
        SceneObject*& selectedObject,
        Light*& selectedLight
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
        Light*& selectedLight,
        Mesh* cubeMesh,
        Shader* shader,
        Material* material,
        Camera& camera,
        Model* torchModel,
        int& lightCounter,
        AppMode& appMode
    );
    static void DrawStatistics(
        Scene& scene,
        Camera& camera,
        SceneObject* selectedObject,
        float deltaTime
    );
    static void DrawLightInspector(
        Light* selectedLight
    );
};