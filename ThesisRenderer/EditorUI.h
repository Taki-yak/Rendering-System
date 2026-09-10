#pragma once

#include "imgui.h"
#include "Scene.h"
#include "Camera.h"
#include "AppMode.h"
#include "Light.h"
#include "Model.h"
#include <functional>
#include "Camera.h"
class EditorUI
{
public:
    static void DrawAssetBrowser(
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
    );
    static void DrawCrosshair();
    static void DrawHierarchy(
        Scene& scene,
        SceneObject*& selectedObject,
        Light*& selectedLight,
        Shader* shader,
        Camera& camera
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
        AppMode& appMode,
        std::function<void()> saveEditorCallback,
        std::function<void()> loadEditorCallback
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