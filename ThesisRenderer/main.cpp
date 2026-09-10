#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "Texture.h"
#include "Model.h"
#include "Camera.h"
#include "Cubemap.h"
#include "Frustum.h"
#include "Component.h"
#include "RotatorComponent.h"
#include "OscillatorComponent.h"
#include "SceneSerializer.h"
#include "GameplayFeedbackFX.h"
#include "ObjectiveMarkerSystem.h"
#include "WeatherSystem.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "InputManager.h"
#include "GridRenderer.h"
#include "EditorUI.h"
#include "RotateComponent.h"
#include "AppMode.h"
#include "PlayerController.h"
#include "AssetDatabase.h"
#include "ThirdPersonController.h"
#include "AnimationLibrary.h"
#include "AnimatedModel.h"
#include "MiniMapRadar.h"
#include "CinematicOverlay.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "AudioSystem.h"
#include <unordered_map>
#include "DayNightSystem.h"
#include "CoinRushGameMode.h"
#include "MonsterEscapeGameMode.h"
#include "EditorLayout.h"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
// ================= CAMERA VARIABLES =================
//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//float yaw = -90.0f;
//float pitch = 0.0f;
float LerpAngleDegrees(
    float current,
    float target,
    float factor
)
{
    float difference =
        std::fmod(
            target - current + 540.0f,
            360.0f
        ) - 180.0f;

    return
        current +
        difference *
        factor;
}
void ConfigureRuntimeAnimationClip(
    AnimatedModel* model,
    const std::string& clipName
)
{
    if (model == nullptr)
        return;

    model->SetRemoveRootMotion(
        true
    );

    if (clipName == "Idle")
    {
        model->SetLooping(
            true
        );

        model->SetAnimationSpeed(
            0.35f
        );
    }
    else if (clipName == "Walk")
    {
        model->SetLooping(
            true
        );

        model->SetAnimationSpeed(
            0.55f
        );
    }
    else if (clipName == "Run")
    {
        model->SetLooping(
            true
        );

        model->SetAnimationSpeed(
            0.70f
        );
    }
    else if (clipName == "Jump")
    {
        model->SetLooping(
            false
        );

        model->SetAnimationSpeed(
            1.5f
        );
    }
}
GLuint LoadMenuTexture(
    const char* path
)
{
    int width =
        0;

    int height =
        0;

    int channels =
        0;

    stbi_set_flip_vertically_on_load(
        false
    );

    unsigned char* data =
        stbi_load(
            path,
            &width,
            &height,
            &channels,
            0
        );

    if (data == nullptr)
    {
        std::cout
            << "Failed to load menu texture: "
            << path
            << std::endl;

        return 0;
    }

    GLenum format =
        GL_RGB;

    if (channels == 4)
    {
        format =
            GL_RGBA;
    }

    GLuint textureId =
        0;

    glGenTextures(
        1,
        &textureId
    );

    glBindTexture(
        GL_TEXTURE_2D,
        textureId
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    glGenerateMipmap(
        GL_TEXTURE_2D
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    stbi_image_free(
        data
    );

    return textureId;
}

glm::vec3 GetRayFromMouse(double mouseX, double mouseY, int width, int height,
    glm::mat4 projection, glm::mat4 view)
{
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    float z = 1.0f;

    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world);

    return ray_world;
}

bool isSelected;
bool useFrustumCulling = true;
double previousTime = glfwGetTime();
int frameCount = 0;
float lastX = 400;
float lastY = 300;
bool firstMouse = true;
void TestAssimp();
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool useCulling = true;
bool useDistanceOptimization =
true;
// ================= MAIN MENU =================
bool showMainMenu =
true;

bool mainMenuClickPlayed =
false;

bool mainMenuLinkClicked =
false;

bool escapeMenuPressed =
false;

bool mainMenuMusicPlaying =
false;

GLuint mainMenuBackgroundTexture =
0;

const char* mainMenuGithubUrl =
"https://github.com/Taki-yak";

const char* mainMenuEmailUrl =
"https://www.linkedin.com/in/taki-eddine-yakhlef-586730293/";
bool useTorchFireFlicker =
true;
float tinyPropRenderDistance =
30.0f;
float smallPropRenderDistance =
75.0f;
float rockRenderDistance =
115.0f;
float mediumPropRenderDistance =180.0f;
int distanceCulledObjects =
0;
bool cKeyPressed = false;
bool nKeyPressed = false;
bool nKeyLastState = false;
bool mKeyPressed = false;
bool mouseClicked = false;
bool pPressed = false;
bool lPressed = false;
bool bPressed = false;
bool tabModeSwitchPressed =
false;
bool isDragging = false;
bool snapEnabled = false;
bool gPressed = false;
float gridSize = 1.0f;
bool useGridSnap = true;
bool blockEditorMouseLook =
false;
float playCameraLookOffset =0.0f;
bool rightMouseCameraActive =
false;
bool interactionKeyPressed =
false;
int interactionCount =0;
bool walkingFootstepPlaying =
false;

bool runningFootstepPlaying =
false;
float nearbyInteractableDistance =-1.0f;
bool musicRescueActive =
    false;

bool musicNpcChasingMonster =
    false;

bool musicRescueSoundStarted =
    false;

bool monsterDefeatedByMusic =
    false;

bool musicRescueWin =
    false;

float musicGateRadius =
    4.5f;

float musicNpcSpeed =
    8.0f;

float musicNpcCatchRadius =
    2.4f;
int mainMenuHoveredButton =
-1;

int previousMainMenuHoveredButton =
-1;
float musicNpcTerrainOffset =
    0.15f;
bool showVisualPolishPanel =
true;
bool showUIPanelsWindow =
true;

bool showHierarchyPanel =
true;

bool showInspectorPanel =
true;

bool showAssetBrowserPanel =
true;
bool showPlayerToolsPanel =
true;
bool showStatisticsPanel =
true;

bool showLightInspectorPanel =
false;

bool showDebugPanel =
false;

bool showSelectedObjectToolsPanel =
false;
bool showCollisionDebug =
false;

float collisionDebugYOffset =
0.08f;

float collisionDebugLineWidth =
2.0f;
MiniMapRadar miniMapRadar;
CinematicOverlay cinematicOverlay;
GameplayFeedbackFX gameplayFeedbackFX;
ObjectiveMarkerSystem objectiveMarkerSystem;
WeatherSystem weatherSystem;
DayNightSystem dayNightSystem;
CoinRushGameMode coinRushGameMode;
MonsterEscapeGameMode monsterEscapeGameMode;
enum class RuntimeGameMode
{
    FreeRoam = 0,
    CoinRush = 1,
    MonsterEscape = 2,
    FullDemo = 3
};

RuntimeGameMode selectedRuntimeGameMode =
RuntimeGameMode::FullDemo;

bool showGameModeSelectorWindow =
true;

const char* runtimeGameModeNames[] =
{
    "Free Roam",
    "Coin Rush",
    "Monster Escape",
    "Full Demo"
};

bool IsCoinRushModeEnabled()
{
    return
        selectedRuntimeGameMode == RuntimeGameMode::CoinRush ||
        selectedRuntimeGameMode == RuntimeGameMode::FullDemo;
}

bool IsMonsterEscapeModeEnabled()
{
    return
        selectedRuntimeGameMode == RuntimeGameMode::MonsterEscape ||
        selectedRuntimeGameMode == RuntimeGameMode::FullDemo;
}

bool IsMusicRescueModeEnabled()
{
    return
        selectedRuntimeGameMode == RuntimeGameMode::FullDemo;
}

const char* GetSelectedRuntimeGameModeName()
{
    return
        runtimeGameModeNames[
            static_cast<int>(
                selectedRuntimeGameMode
                )
        ];
}

bool ShouldLockPlayerAfterRuntimeResult()
{
    if (
        IsCoinRushModeEnabled() &&
        (
            coinRushGameMode.won ||
            coinRushGameMode.lost
            )
        )
    {
        return true;
    }

    if (
        IsMonsterEscapeModeEnabled() &&
        monsterEscapeGameMode.playerCaught
        )
    {
        return true;
    }

    if (
        IsMusicRescueModeEnabled() &&
        musicRescueWin
        )
    {
        return true;
    }

    return false;
}
std::string musicRescueText =
    "Music Rescue: Waiting for gate.";
float interactionRadius =4.0f;
SceneObject* nearbyInteractableObject =
nullptr;

std::string interactionHintText =
"";

std::string interactionResultText =
"Walk near an object and press E.";

float interactionResultTimer =
0.0f;

bool ignoreNextMouseDelta =
false;
bool IsEditorSavedObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->name == "Player")
        return false;

    if (object->name == "Procedural Terrain")
        return false;

    if (object->name.find("Generated") != std::string::npos)
        return false;

    if (!object->persistent)
        return false;

    if (object->spawnSource != SpawnSource::Manual)
        return false;

    return true;
}
std::string GetEditorObjectType(SceneObject* object)
{
    if (object == nullptr)
        return "Unknown";

    std::string name =
        object->name;

    // ================= HOUSES =================
    if (name.find("Camp House 2") != std::string::npos)
        return "House2";

    if (name.find("House 2") != std::string::npos)
        return "House2";

    if (name.find("Camp House 1") != std::string::npos)
        return "House1";

    if (name.find("House 1") != std::string::npos)
        return "House1";

    if (name.find("Wooden House") != std::string::npos)
        return "House1";

    if (name.find("Camp House") != std::string::npos)
        return "House1";

    // ================= NATURE =================
    if (name.find("Pine Tree") != std::string::npos)
        return "PineTree";

    if (name.find("Camp Tree") != std::string::npos)
        return "PineTree";

    if (name.find("Common Tree") != std::string::npos)
        return "CommonTree";

    if (name.find("Rock") != std::string::npos)
        return "Rock";

    if (name.find("Bush") != std::string::npos)
        return "Bush";

    if (name.find("Wood Log") != std::string::npos)
        return "WoodLog";

    if (name.find("Camp Log") != std::string::npos)
        return "WoodLog";

    if (name.find("Tree Stump") != std::string::npos)
        return "TreeStump";

    if (name.find("Camp Stump") != std::string::npos)
        return "TreeStump";

    if (name.find("Grass") != std::string::npos)
        return "Grass";

  
    if (name.find("Stone Wall") != std::string::npos)
        return "StoneWall";

    if (name.find("Wall") != std::string::npos)
        return "StoneWall";

    if (name.find("Wood Fence Segment") != std::string::npos)
        return "WoodFenceSegment";

    if (name.find("Fence Segment") != std::string::npos)
        return "WoodFenceSegment";

    if (name.find("Wood Fence Post") != std::string::npos)
        return "WoodFencePost";

    if (name.find("Fence Post") != std::string::npos)
        return "WoodFencePost";

    if (name.find("Dirt Path Tile") != std::string::npos)
        return "DirtPathTile";

    if (name.find("Path Tile") != std::string::npos)
        return "DirtPathTile";

    if (name.find("Wood Platform") != std::string::npos)
        return "WoodPlatform";

    if (name.find("Platform") != std::string::npos)
        return "WoodPlatform";
    if (name.find("Campfire Log") != std::string::npos)
        return "CampfireLog";

    if (name.find("Campfire") != std::string::npos)
        return "Campfire";
    return "Cube";
}

void SaveEditorObjects(
    Scene& scene,
    const std::string& filePath
)
{
    std::ofstream file(
        filePath
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to save editor objects."
            << std::endl;

        return;
    }

    for (SceneObject* object : scene.objects)
    {
        if (!IsEditorSavedObject(object))
            continue;

        Material* objectMaterial =
            object->material;

        bool hasMaterial =
            objectMaterial != nullptr;

        glm::vec3 tint =
            hasMaterial ?
            objectMaterial->tint :
            glm::vec3(
                1.0f
            );

        glm::vec3 ambient =
            hasMaterial ?
            objectMaterial->ambient :
            glm::vec3(
                0.35f
            );

        glm::vec3 diffuse =
            hasMaterial ?
            objectMaterial->diffuse :
            glm::vec3(
                0.75f
            );

        glm::vec3 specular =
            hasMaterial ?
            objectMaterial->specular :
            glm::vec3(
                0.04f
            );

        float shininess =
            hasMaterial ?
            objectMaterial->shininess :
            6.0f;

        bool wireframe =
            hasMaterial ?
            objectMaterial->wireframe :
            false;

        file
            << "V3" << "|"

            // identity
            << object->name << "|"
            << object->editorMeshType << "|"
            << object->editorModelPath << "|"
            << object->editorModelDirectory << "|"
            << object->editorTexturePath << "|"
            << object->editorGameplayType << "|"
            << object->editorPrimitiveDetail << "|"

            // transform
            << object->transform.position.x << " "
            << object->transform.position.y << " "
            << object->transform.position.z << "|"

            << object->transform.rotation.x << " "
            << object->transform.rotation.y << " "
            << object->transform.rotation.z << "|"

            << object->transform.scale.x << " "
            << object->transform.scale.y << " "
            << object->transform.scale.z << "|"

            // object state
            << object->visible << "|"
            << object->isCollider << "|"
            << object->colliderRadius << "|"
            << object->boundingRadius << "|"

            // material state
            << hasMaterial << "|"

            << tint.x << " "
            << tint.y << " "
            << tint.z << "|"

            << ambient.x << " "
            << ambient.y << " "
            << ambient.z << "|"

            << diffuse.x << " "
            << diffuse.y << " "
            << diffuse.z << "|"

            << specular.x << " "
            << specular.y << " "
            << specular.z << "|"

            << shininess << "|"
            << wireframe

            << "\n";
    }

    file.close();

    std::cout
        << "Generic editor scene saved to "
        << filePath
        << std::endl;
}
Model* GetModelFromEditorType(
    const std::string& objectType,
    Model* woodenHouseModel,
    Model* newHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel
)
{
    if (
        objectType == "House1" ||
        objectType == "WoodenHouse"
        )
    {
        return woodenHouseModel;
    }

    if (objectType == "House2")
        return newHouseModel;

    if (objectType == "PineTree")
        return pineTreeModel;

    if (objectType == "CommonTree")
        return commonTreeModel;

    if (objectType == "Rock")
        return rockModel;

    if (objectType == "Bush")
        return bushModel;

    if (objectType == "WoodLog")
        return woodLogModel;

    if (objectType == "TreeStump")
        return treeStumpModel;

    if (objectType == "Grass")
        return grassModel;

    return nullptr;
}
Material* CreateLoadedCubeMaterial(
    const std::string& objectType
)
{
    Material* objectMaterial =
        new Material(
            nullptr
        );

    objectMaterial->tint =
        glm::vec3(
            0.75f,
            0.75f,
            0.75f
        );

    objectMaterial->ambient =
        glm::vec3(
            0.35f,
            0.35f,
            0.35f
        );

    objectMaterial->diffuse =
        glm::vec3(
            0.75f,
            0.75f,
            0.75f
        );

    objectMaterial->specular =
        glm::vec3(
            0.04f,
            0.04f,
            0.04f
        );

    objectMaterial->shininess =
        6.0f;

    if (objectType == "StoneWall")
    {
        objectMaterial->tint =
            glm::vec3(
                0.42f,
                0.40f,
                0.36f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.35f,
                0.35f,
                0.35f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.75f,
                0.72f,
                0.66f
            );
    }

    if (objectType == "WoodFenceSegment")
    {
        objectMaterial->tint =
            glm::vec3(
                0.45f,
                0.28f,
                0.13f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.32f,
                0.25f,
                0.18f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.75f,
                0.50f,
                0.25f
            );
    }

    if (objectType == "WoodFencePost")
    {
        objectMaterial->tint =
            glm::vec3(
                0.34f,
                0.20f,
                0.10f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.28f,
                0.20f,
                0.14f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.65f,
                0.42f,
                0.22f
            );
    }

    if (objectType == "DirtPathTile")
    {
        objectMaterial->tint =
            glm::vec3(
                0.42f,
                0.31f,
                0.18f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.32f,
                0.25f,
                0.18f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.70f,
                0.52f,
                0.30f
            );
    }

    if (objectType == "WoodPlatform")
    {
        objectMaterial->tint =
            glm::vec3(
                0.48f,
                0.30f,
                0.14f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.32f,
                0.24f,
                0.16f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.78f,
                0.52f,
                0.28f
            );
    }
    if (objectType == "CampfireLog")
    {
        objectMaterial->tint =
            glm::vec3(
                0.42f,
                0.24f,
                0.10f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.22f,
                0.14f,
                0.08f
            );

        objectMaterial->diffuse =
            glm::vec3(
                0.65f,
                0.38f,
                0.18f
            );

        objectMaterial->specular =
            glm::vec3(
                0.05f,
                0.04f,
                0.03f
            );

        objectMaterial->shininess =
            4.0f;
    }

    if (objectType == "Campfire")
    {
        objectMaterial->tint =
            glm::vec3(
                1.0f,
                0.35f,
                0.05f
            );

        objectMaterial->ambient =
            glm::vec3(
                0.75f,
                0.22f,
                0.04f
            );

        objectMaterial->diffuse =
            glm::vec3(
                1.0f,
                0.45f,
                0.08f
            );

        objectMaterial->specular =
            glm::vec3(
                0.08f,
                0.05f,
                0.02f
            );

        objectMaterial->shininess =
            4.0f;
    }
    return objectMaterial;
}
glm::vec3 GetTorchLightOnColor();
static std::vector<std::string> SplitEditorSaveLine(
    const std::string& line
)
{
    std::vector<std::string> parts;
    std::stringstream stream(line);
    std::string part;

    while (std::getline(stream, part, '|'))
    {
        parts.push_back(part);
    }

    return parts;
}

static glm::vec3 ParseEditorVec3(
    const std::string& text,
    const glm::vec3& fallback = glm::vec3(0.0f)
)
{
    std::stringstream stream(text);

    glm::vec3 value =
        fallback;

    stream
        >> value.x
        >> value.y
        >> value.z;

    return value;
}

static Texture* GetLoadedEditorTexture(
    const std::string& path
)
{
    if (path.empty())
        return nullptr;

    static std::unordered_map<std::string, Texture*> loadedTextures;

    auto it =
        loadedTextures.find(path);

    if (it != loadedTextures.end())
        return it->second;

    Texture* texture =
        new Texture(
            path.c_str()
        );

    loadedTextures[path] =
        texture;

    return texture;
}

static void AddPrimitiveVertexMain(
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

static Mesh* CreateLoadedSphereMesh(
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
        float v0 = (float)r / (float)rings;
        float v1 = (float)(r + 1) / (float)rings;

        float phi0 = v0 * pi;
        float phi1 = v1 * pi;

        for (int s = 0; s < sectors; s++)
        {
            float u0 = (float)s / (float)sectors;
            float u1 = (float)(s + 1) / (float)sectors;

            float theta0 = u0 * pi * 2.0f;
            float theta1 = u1 * pi * 2.0f;

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

            AddPrimitiveVertexMain(data, p00, glm::normalize(p00), glm::vec2(u0, v0));
            AddPrimitiveVertexMain(data, p10, glm::normalize(p10), glm::vec2(u0, v1));
            AddPrimitiveVertexMain(data, p01, glm::normalize(p01), glm::vec2(u1, v0));

            AddPrimitiveVertexMain(data, p01, glm::normalize(p01), glm::vec2(u1, v0));
            AddPrimitiveVertexMain(data, p10, glm::normalize(p10), glm::vec2(u0, v1));
            AddPrimitiveVertexMain(data, p11, glm::normalize(p11), glm::vec2(u1, v1));
        }
    }

    return new Mesh(
        data.data(),
        static_cast<int>(data.size() * sizeof(float))
    );
}

static Mesh* CreateLoadedCylinderMesh(
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
        float u0 = (float)i / (float)sectors;
        float u1 = (float)(i + 1) / (float)sectors;

        float a0 = u0 * pi * 2.0f;
        float a1 = u1 * pi * 2.0f;

        glm::vec3 b0(std::cos(a0) * radius, -halfHeight, std::sin(a0) * radius);
        glm::vec3 b1(std::cos(a1) * radius, -halfHeight, std::sin(a1) * radius);
        glm::vec3 t0(std::cos(a0) * radius, halfHeight, std::sin(a0) * radius);
        glm::vec3 t1(std::cos(a1) * radius, halfHeight, std::sin(a1) * radius);

        glm::vec3 n0 =
            glm::normalize(glm::vec3(b0.x, 0.0f, b0.z));

        glm::vec3 n1 =
            glm::normalize(glm::vec3(b1.x, 0.0f, b1.z));

        AddPrimitiveVertexMain(data, b0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertexMain(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, t0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertexMain(data, t0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertexMain(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, t1, n1, glm::vec2(u1, 1.0f));

        glm::vec3 bottomCenter(0.0f, -halfHeight, 0.0f);
        glm::vec3 topCenter(0.0f, halfHeight, 0.0f);

        AddPrimitiveVertexMain(data, bottomCenter, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f));
        AddPrimitiveVertexMain(data, b1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
        AddPrimitiveVertexMain(data, b0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));

        AddPrimitiveVertexMain(data, topCenter, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.5f));
        AddPrimitiveVertexMain(data, t0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, t1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(data.size() * sizeof(float))
    );
}

static Mesh* CreateLoadedConeMesh(
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

    glm::vec3 top(0.0f, halfHeight, 0.0f);
    glm::vec3 bottomCenter(0.0f, -halfHeight, 0.0f);

    for (int i = 0; i < sectors; i++)
    {
        float u0 = (float)i / (float)sectors;
        float u1 = (float)(i + 1) / (float)sectors;

        float a0 = u0 * pi * 2.0f;
        float a1 = u1 * pi * 2.0f;

        glm::vec3 b0(std::cos(a0) * radius, -halfHeight, std::sin(a0) * radius);
        glm::vec3 b1(std::cos(a1) * radius, -halfHeight, std::sin(a1) * radius);

        glm::vec3 sideNormal =
            glm::normalize(
                glm::cross(
                    b1 - b0,
                    top - b0
                )
            );

        AddPrimitiveVertexMain(data, b0, sideNormal, glm::vec2(u0, 0.0f));
        AddPrimitiveVertexMain(data, b1, sideNormal, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, top, sideNormal, glm::vec2(0.5f, 1.0f));

        AddPrimitiveVertexMain(data, bottomCenter, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.5f));
        AddPrimitiveVertexMain(data, b0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, b1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(data.size() * sizeof(float))
    );
}
static void AddLoadedBoxToMesh(
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

    AddPrimitiveVertexMain(data, p001, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p101, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p111, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p001, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p111, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p011, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f));

    AddPrimitiveVertexMain(data, p100, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p000, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p010, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p100, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p010, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p110, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f));

    AddPrimitiveVertexMain(data, p000, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p001, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p011, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p000, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p011, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p010, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    AddPrimitiveVertexMain(data, p101, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p100, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p110, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p101, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p110, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p111, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    AddPrimitiveVertexMain(data, p010, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p011, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p111, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p010, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p111, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p110, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

    AddPrimitiveVertexMain(data, p000, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p100, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, p101, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p000, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, p101, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
    AddPrimitiveVertexMain(data, p001, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
}

static Mesh* CreateLoadedStairsMesh(
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

        AddLoadedBoxToMesh(
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
static Mesh* CreateLoadedRingMesh(
    int majorSegments,
    int minorSegments
)
{
    if (majorSegments < 6)
        majorSegments = 6;

    if (minorSegments < 4)
        minorSegments = 4;

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

            glm::vec3 p00(
                (majorRadius + minorRadius * std::cos(v0)) * std::cos(u0),
                minorRadius * std::sin(v0),
                (majorRadius + minorRadius * std::cos(v0)) * std::sin(u0)
            );

            glm::vec3 p01(
                (majorRadius + minorRadius * std::cos(v1)) * std::cos(u0),
                minorRadius * std::sin(v1),
                (majorRadius + minorRadius * std::cos(v1)) * std::sin(u0)
            );

            glm::vec3 p10(
                (majorRadius + minorRadius * std::cos(v0)) * std::cos(u1),
                minorRadius * std::sin(v0),
                (majorRadius + minorRadius * std::cos(v0)) * std::sin(u1)
            );

            glm::vec3 p11(
                (majorRadius + minorRadius * std::cos(v1)) * std::cos(u1),
                minorRadius * std::sin(v1),
                (majorRadius + minorRadius * std::cos(v1)) * std::sin(u1)
            );

            glm::vec3 n00 =
                glm::normalize(p00 - glm::vec3(majorRadius * std::cos(u0), 0.0f, majorRadius * std::sin(u0)));

            glm::vec3 n01 =
                glm::normalize(p01 - glm::vec3(majorRadius * std::cos(u0), 0.0f, majorRadius * std::sin(u0)));

            glm::vec3 n10 =
                glm::normalize(p10 - glm::vec3(majorRadius * std::cos(u1), 0.0f, majorRadius * std::sin(u1)));

            glm::vec3 n11 =
                glm::normalize(p11 - glm::vec3(majorRadius * std::cos(u1), 0.0f, majorRadius * std::sin(u1)));

            AddPrimitiveVertexMain(data, p00, n00, glm::vec2(0.0f, 0.0f));
            AddPrimitiveVertexMain(data, p10, n10, glm::vec2(1.0f, 0.0f));
            AddPrimitiveVertexMain(data, p01, n01, glm::vec2(0.0f, 1.0f));

            AddPrimitiveVertexMain(data, p01, n01, glm::vec2(0.0f, 1.0f));
            AddPrimitiveVertexMain(data, p10, n10, glm::vec2(1.0f, 0.0f));
            AddPrimitiveVertexMain(data, p11, n11, glm::vec2(1.0f, 1.0f));
        }
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static Mesh* CreateLoadedPipeMesh(
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

        AddPrimitiveVertexMain(data, ob0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertexMain(data, ob1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, ot0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertexMain(data, ot0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertexMain(data, ob1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, ot1, n1, glm::vec2(u1, 1.0f));

        AddPrimitiveVertexMain(data, ib1, -n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, ib0, -n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertexMain(data, it0, -n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertexMain(data, ib1, -n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, it0, -n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertexMain(data, it1, -n1, glm::vec2(u1, 1.0f));

        AddPrimitiveVertexMain(data, it0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, ot0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
        AddPrimitiveVertexMain(data, ot1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

        AddPrimitiveVertexMain(data, it0, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, ot1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
        AddPrimitiveVertexMain(data, it1, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f));

        AddPrimitiveVertexMain(data, ib0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, ib1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f));
        AddPrimitiveVertexMain(data, ob1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));

        AddPrimitiveVertexMain(data, ib0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f));
        AddPrimitiveVertexMain(data, ob1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f));
        AddPrimitiveVertexMain(data, ob0, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f));
    }

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static Mesh* CreateLoadedCapsuleMesh(
    int rings,
    int sectors
)
{
    if (rings < 4)
        rings = 4;

    if (sectors < 8)
        sectors = 8;

    std::vector<float> data;

    const float pi =
        3.14159265359f;

    float radius =
        0.28f;

    float halfBody =
        0.25f;

    for (int i = 0; i < sectors; i++)
    {
        float u0 = (float)i / (float)sectors;
        float u1 = (float)(i + 1) / (float)sectors;

        float a0 = u0 * pi * 2.0f;
        float a1 = u1 * pi * 2.0f;

        glm::vec3 b0(std::cos(a0) * radius, -halfBody, std::sin(a0) * radius);
        glm::vec3 b1(std::cos(a1) * radius, -halfBody, std::sin(a1) * radius);
        glm::vec3 t0(std::cos(a0) * radius, halfBody, std::sin(a0) * radius);
        glm::vec3 t1(std::cos(a1) * radius, halfBody, std::sin(a1) * radius);

        glm::vec3 n0 = glm::normalize(glm::vec3(std::cos(a0), 0.0f, std::sin(a0)));
        glm::vec3 n1 = glm::normalize(glm::vec3(std::cos(a1), 0.0f, std::sin(a1)));

        AddPrimitiveVertexMain(data, b0, n0, glm::vec2(u0, 0.0f));
        AddPrimitiveVertexMain(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, t0, n0, glm::vec2(u0, 1.0f));

        AddPrimitiveVertexMain(data, t0, n0, glm::vec2(u0, 1.0f));
        AddPrimitiveVertexMain(data, b1, n1, glm::vec2(u1, 0.0f));
        AddPrimitiveVertexMain(data, t1, n1, glm::vec2(u1, 1.0f));
    }

    // Simpler loaded capsule: body + smooth enough visual using sphere mesh style fallback.
    // For save/load stability, this preserves the capsule mesh type.
    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}

static void AddLoadedTriangleToMesh(
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

    AddPrimitiveVertexMain(data, a, normal, glm::vec2(0.0f, 0.0f));
    AddPrimitiveVertexMain(data, b, normal, glm::vec2(1.0f, 0.0f));
    AddPrimitiveVertexMain(data, c, normal, glm::vec2(0.5f, 1.0f));
}

static void AddLoadedQuadToMesh(
    std::vector<float>& data,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& c,
    const glm::vec3& d
)
{
    AddLoadedTriangleToMesh(
        data,
        a,
        b,
        c
    );

    AddLoadedTriangleToMesh(
        data,
        a,
        c,
        d
    );
}

static Mesh* CreateLoadedRoofWedgeMesh()
{
    std::vector<float> data;

    glm::vec3 a(-0.5f, -0.5f, -0.5f);
    glm::vec3 b(0.5f, -0.5f, -0.5f);
    glm::vec3 c(0.5f, -0.5f, 0.5f);
    glm::vec3 d(-0.5f, -0.5f, 0.5f);

    glm::vec3 e(-0.5f, 0.5f, 0.0f);
    glm::vec3 f(0.5f, 0.5f, 0.0f);

    AddLoadedQuadToMesh(data, a, b, c, d);
    AddLoadedQuadToMesh(data, a, e, f, b);
    AddLoadedQuadToMesh(data, d, c, f, e);
    AddLoadedTriangleToMesh(data, a, d, e);
    AddLoadedTriangleToMesh(data, b, f, c);

    return new Mesh(
        data.data(),
        static_cast<int>(
            data.size() * sizeof(float)
            )
    );
}
static Mesh* GetLoadedProceduralMesh(
    const std::string& meshType
)
{
    static Mesh* sphereMesh = nullptr;
    static Mesh* cylinderMesh = nullptr;
    static Mesh* coneMesh = nullptr;
    static Mesh* stairsMesh = nullptr;
    static Mesh* ringMesh = nullptr;
    static Mesh* pipeMesh = nullptr;
    static Mesh* capsuleMesh = nullptr;
    static Mesh* roofWedgeMesh = nullptr;
    if (meshType == "Sphere")
    {
        if (sphereMesh == nullptr)
            sphereMesh = CreateLoadedSphereMesh(24, 32);

        return sphereMesh;
    }

    if (meshType == "Cylinder")
    {
        if (cylinderMesh == nullptr)
            cylinderMesh = CreateLoadedCylinderMesh(32);

        return cylinderMesh;
    }

    if (meshType == "Cone")
    {
        if (coneMesh == nullptr)
            coneMesh = CreateLoadedConeMesh(32);

        return coneMesh;
    }
    if (meshType == "Stairs")
    {
        if (stairsMesh == nullptr)
            stairsMesh =
            CreateLoadedStairsMesh(
                5
            );

        return stairsMesh;
    }
    if (meshType == "Ring")
    {
        if (ringMesh == nullptr)
        {
            ringMesh =
                CreateLoadedRingMesh(
                    32,
                    12
                );
        }

        return ringMesh;
    }

    if (meshType == "Pipe")
    {
        if (pipeMesh == nullptr)
        {
            pipeMesh =
                CreateLoadedPipeMesh(
                    32
                );
        }

        return pipeMesh;
    }
    if (meshType == "Capsule")
    {
        if (capsuleMesh == nullptr)
        {
            capsuleMesh =
                CreateLoadedCapsuleMesh(
                    12,
                    32
                );
        }

        return capsuleMesh;
    }

    if (meshType == "RoofWedge")
    {
        if (roofWedgeMesh == nullptr)
        {
            roofWedgeMesh =
                CreateLoadedRoofWedgeMesh();
        }

        return roofWedgeMesh;
    }
    return nullptr;
}static int ClampLoadedPrimitiveInt(
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

static bool ParseLoadedPrimitiveDetail(
    const std::string& detail,
    int& sphereRings,
    int& sphereSectors,
    int& cylinderSides,
    int& coneSides
)
{
    if (detail.empty())
        return false;

    std::stringstream stream(
        detail
    );

    if (
        !(
            stream
            >> sphereRings
            >> sphereSectors
            >> cylinderSides
            >> coneSides
            )
        )
    {
        return false;
    }

    sphereRings =
        ClampLoadedPrimitiveInt(
            sphereRings,
            4,
            64
        );

    sphereSectors =
        ClampLoadedPrimitiveInt(
            sphereSectors,
            6,
            96
        );

    cylinderSides =
        ClampLoadedPrimitiveInt(
            cylinderSides,
            3,
            96
        );

    coneSides =
        ClampLoadedPrimitiveInt(
            coneSides,
            3,
            96
        );

    return true;
}

static Mesh* CreateLoadedPrimitiveMeshFromDetail(
    const std::string& meshType,
    const std::string& primitiveDetail
)
{
    int sphereRings =
        24;

    int sphereSectors =
        32;

    int cylinderSides =
        32;

    int coneSides =
        32;
    // ================= LOAD STAIRS DETAIL V3A =================
    if (meshType == "Stairs")
    {
        int stairsSteps =
            5;

        std::string prefix =
            "stairs_steps=";

        if (
            primitiveDetail.find(
                prefix
            ) == 0
            )
        {
            std::string valueText =
                primitiveDetail.substr(
                    prefix.size()
                );

            stairsSteps =
                std::atoi(
                    valueText.c_str()
                );
        }

        stairsSteps =
            ClampLoadedPrimitiveInt(
                stairsSteps,
                2,
                12
            );

        return CreateLoadedStairsMesh(
            stairsSteps
        );
    }
    if (
        !ParseLoadedPrimitiveDetail(
            primitiveDetail,
            sphereRings,
            sphereSectors,
            cylinderSides,
            coneSides
        )
        )
    {
        return nullptr;
    }

    if (meshType == "Sphere")
    {
        return CreateLoadedSphereMesh(
            sphereRings,
            sphereSectors
        );
    }

    if (meshType == "Cylinder")
    {
        return CreateLoadedCylinderMesh(
            cylinderSides
        );
    }

    if (meshType == "Cone")
    {
        return CreateLoadedConeMesh(
            coneSides
        );
    }

    return nullptr;
}
static Model* GetLoadedEditorModel(
    const std::string& modelPath,
    const std::string& modelDirectory
)
{
    if (modelPath.empty())
        return nullptr;

    static std::unordered_map<std::string, Model*> loadedModels;

    auto it =
        loadedModels.find(modelPath);

    if (it != loadedModels.end())
        return it->second;

    Model* model =
        new Model(
            modelPath,
            modelDirectory
        );

    loadedModels[modelPath] =
        model;

    return model;
}

static Material* CreateLoadedV2Material(
    bool hasMaterial,
    const std::string& texturePath,
    const std::string& tintPart,
    const std::string& ambientPart,
    const std::string& diffusePart,
    const std::string& specularPart,
    const std::string& shininessPart,
    const std::string& wireframePart
)
{
    Material* material =
        new Material(
            nullptr
        );

    material->tint =
        glm::vec3(1.0f);

    material->ambient =
        glm::vec3(0.35f);

    material->diffuse =
        glm::vec3(0.75f);

    material->specular =
        glm::vec3(0.04f);

    material->shininess =
        6.0f;

    material->wireframe =
        false;

    if (hasMaterial)
    {
        material->tint =
            ParseEditorVec3(
                tintPart,
                glm::vec3(1.0f)
            );

        material->ambient =
            ParseEditorVec3(
                ambientPart,
                glm::vec3(0.35f)
            );

        material->diffuse =
            ParseEditorVec3(
                diffusePart,
                glm::vec3(0.75f)
            );

        material->specular =
            ParseEditorVec3(
                specularPart,
                glm::vec3(0.04f)
            );

        if (!shininessPart.empty())
        {
            material->shininess =
                std::stof(
                    shininessPart
                );
        }

        material->wireframe =
            wireframePart == "1";
    }

    material->texture =
        GetLoadedEditorTexture(
            texturePath
        );

    return material;
}
void LoadEditorObjects(
    Scene& scene,
    const std::string& filePath,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    SceneObject*& selectedObject,
    Model* woodenHouseModel,
    Model* newHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel
)
{
    (void)material;
    (void)woodenHouseModel;
    (void)newHouseModel;
    (void)pineTreeModel;
    (void)commonTreeModel;
    (void)rockModel;
    (void)bushModel;
    (void)woodLogModel;
    (void)treeStumpModel;
    (void)grassModel;

    std::ifstream file(
        filePath
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to load editor objects."
            << std::endl;

        return;
    }

    for (auto it = scene.objects.begin(); it != scene.objects.end(); )
    {
        SceneObject* object =
            *it;

        if (
            object != nullptr &&
            IsEditorSavedObject(object)
            )
        {
         

            it =
                scene.objects.erase(
                    it
                );
        }
        else
        {
            ++it;
        }
    }

    selectedObject =
        nullptr;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::vector<std::string> parts =
            SplitEditorSaveLine(
                line
            );

        bool isV2Save =
            parts[0] == "V2";

        bool isV3Save =
            parts[0] == "V3";

        if (
            (!isV2Save && !isV3Save) ||
            (isV2Save && parts.size() < 21) ||
            (isV3Save && parts.size() < 22)
            )
        {
            std::cout
                << "Skipped unsupported save line."
                << std::endl;

            continue;
        }

        int dataOffset =
            isV3Save ? 1 : 0;

        std::string namePart =
            parts[1];

        std::string meshType =
            parts[2];

        std::string modelPath =
            parts[3];

        std::string modelDirectory =
            parts[4];

        std::string texturePath =
            parts[5];

        std::string gameplayType =
            parts[6];

        std::string primitiveDetail =
            "";

        if (isV3Save)
        {
            primitiveDetail =
                parts[7];
        }

        glm::vec3 position =
            ParseEditorVec3(
                parts[7 + dataOffset]
            );

        glm::vec3 rotation =
            ParseEditorVec3(
                parts[8 + dataOffset]
            );

        glm::vec3 scale =
            ParseEditorVec3(
                parts[9 + dataOffset],
                glm::vec3(1.0f)
            );

        bool visible =
            parts[10 + dataOffset] == "1";

        bool isCollider =
            parts[11 + dataOffset] == "1";

        float colliderRadius =
            parts[12 + dataOffset].empty()
            ? 1.0f
            : std::stof(
                parts[12 + dataOffset]
            );

        float boundingRadius =
            parts[13 + dataOffset].empty()
            ? 50.0f
            : std::stof(
                parts[13 + dataOffset]
            );

        bool hasMaterial =
            parts[14 + dataOffset] == "1";

        Material* loadedMaterial =
            CreateLoadedV2Material(
                hasMaterial,
                texturePath,
                parts[15 + dataOffset],
                parts[16 + dataOffset],
                parts[17 + dataOffset],
                parts[18 + dataOffset],
                parts[19 + dataOffset],
                parts[20 + dataOffset]
            );

        SceneObject* object =
            nullptr;
        if (meshType == "Model")
        {
            Model* loadedModel =
                GetLoadedEditorModel(
                    modelPath,
                    modelDirectory
                );

            if (loadedModel != nullptr)
            {
                object =
                    new SceneObject(
                        loadedModel,
                        shader
                    );

                object->material =
                    loadedMaterial;
            }
        }
        else
        {
            Mesh* loadedMesh =
                CreateLoadedPrimitiveMeshFromDetail(
                    meshType,
                    primitiveDetail
                );

            if (loadedMesh == nullptr)
            {
                loadedMesh =
                    GetLoadedProceduralMesh(
                        meshType
                    );
            }

            if (loadedMesh == nullptr)
            {
                loadedMesh =
                    cubeMesh;
            }

            object =
                new SceneObject(
                    loadedMesh,
                    shader,
                    loadedMaterial
                );
        }

        if (object == nullptr)
            continue;

        object->name =
            namePart;

        object->transform.position =
            position;

        object->transform.rotation =
            rotation;

        object->transform.scale =
            scale;

        object->visible =
            visible;

        object->isCollider =
            isCollider;

        object->colliderRadius =
            colliderRadius;

        object->boundingRadius =
            boundingRadius;

        object->assetId =
            namePart;

        object->assetType =
            AssetType::Prop;

        object->spawnSource =
            SpawnSource::Manual;

        object->persistent =
            true;

        object->showInHierarchy =
            true;

        object->editorMeshType =
            meshType;

        object->editorModelPath =
            modelPath;

        object->editorModelDirectory =
            modelDirectory;

        object->editorTexturePath =
            texturePath;

        object->editorGameplayType =
            gameplayType;
        object->editorPrimitiveDetail =
            primitiveDetail;

        scene.AddObject(
            object
        );

        selectedObject =
            object;
    }

    file.close();

    std::cout
        << "Generic V2 editor scene loaded from "
        << filePath
        << std::endl;
}
enum MoveAxis
{
    NONE,
    AXIS_X,
    AXIS_Y,
    AXIS_Z
};
enum GizmoMode
{
    TRANSLATE,
    ROTATE,
    SCALE
};
void OpenMainMenuLink(
    const char* url
)
{
    if (url == nullptr)
        return;

    std::string command;

#ifdef _WIN32
    command =
        "start \"\" \"" +
        std::string(url) +
        "\"";
#else
    command =
        "xdg-open \"" +
        std::string(url) +
        "\"";
#endif

    std::system(
        command.c_str()
    );
}
enum class MainMenuAction
{
    None,
    Play,
    Editor,
    Exit
};
bool DrawMainMenuButton(
    const char* id,
    const char* label,
    const ImVec2& position,
    const ImVec2& size,
    ImU32 hoverColor,
    ImU32 borderColor
)
{
    ImGui::SetCursorScreenPos(
        position
    );

    ImGui::InvisibleButton(
        id,
        size
    );

    bool hovered =
        ImGui::IsItemHovered();

    bool clicked =
        ImGui::IsItemClicked();

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    if (hovered)
    {
        ImVec2 minPoint =
            ImVec2(
                position.x,
                position.y
            );

        ImVec2 maxPoint =
            ImVec2(
                position.x + size.x,
                position.y + size.y
            );

        drawList->AddRectFilled(
            minPoint,
            maxPoint,
            hoverColor,
            6.0f
        );

        drawList->AddRect(
            minPoint,
            maxPoint,
            borderColor,
            6.0f,
            0,
            2.0f
        );
    }

    return clicked;
}
bool DrawMainMenuIconButton(
    const char* id,
    const ImVec2& position,
    const ImVec2& size
)
{
    ImGui::SetCursorScreenPos(
        position
    );

    ImGui::InvisibleButton(
        id,
        size
    );

    bool hovered =
        ImGui::IsItemHovered();

    bool clicked =
        ImGui::IsItemClicked();

    if (hovered)
    {
        ImDrawList* drawList =
            ImGui::GetWindowDrawList();

        ImVec2 minPoint =
            ImVec2(
                position.x,
                position.y
            );

        ImVec2 maxPoint =
            ImVec2(
                position.x + size.x,
                position.y + size.y
            );

        drawList->AddRectFilled(
            minPoint,
            maxPoint,
            IM_COL32(
                40,
                160,
                255,
                28
            ),
            8.0f
        );

        drawList->AddRect(
            minPoint,
            maxPoint,
            IM_COL32(
                120,
                230,
                255,
                170
            ),
            8.0f,
            0,
            2.0f
        );
    }

    return clicked;
}
MainMenuAction DrawMainMenuScreen(
    GLuint backgroundTexture
)
{
    ImGuiIO& io =
        ImGui::GetIO();
    mainMenuHoveredButton =
        -1;
    ImGui::SetNextWindowPos(
        ImVec2(
            0.0f,
            0.0f
        ),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        io.DisplaySize,
        ImGuiCond_Always
    );

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin(
        "Main Menu",
        nullptr,
        flags
    );

    ImDrawList* drawList =
        ImGui::GetWindowDrawList();

    ImVec2 screenMin =
        ImVec2(
            0.0f,
            0.0f
        );

    ImVec2 screenMax =
        ImVec2(
            io.DisplaySize.x,
            io.DisplaySize.y
        );

    if (backgroundTexture != 0)
    {
        drawList->AddImage(
            (ImTextureID)(intptr_t)backgroundTexture,
            screenMin,
            screenMax
        );
    }
    else
    {
        drawList->AddRectFilled(
            screenMin,
            screenMax,
            IM_COL32(
                8,
                16,
                28,
                255
            )
        );
    }

    float buttonWidth =
        io.DisplaySize.x * 0.320f;

    float buttonHeight =
        io.DisplaySize.y * 0.105f;

    float buttonX =
        io.DisplaySize.x * 0.360f;

    ImVec2 playPosition =
        ImVec2(
            buttonX,
            io.DisplaySize.y * 0.410f
        );

    ImVec2 editorPosition =
        ImVec2(
            buttonX,
            io.DisplaySize.y * 0.565f
        );

    ImVec2 exitPosition =
        ImVec2(
            buttonX,
            io.DisplaySize.y * 0.700f
        );

    ImVec2 buttonSize =
        ImVec2(
            buttonWidth,
            buttonHeight
        );

    ImU32 hoverFill =
        IM_COL32(
            30,
            150,
            255,
            18
        );

    ImU32 hoverBorder =
        IM_COL32(
            120,
            230,
            255,
            150
        );

    MainMenuAction action =
        MainMenuAction::None;
  
    if (
        DrawMainMenuButton(
            "MainMenu_Play",
            "ENTER PLAY MODE",
            playPosition,
            buttonSize,
            hoverFill,
            hoverBorder
        )
        )
    {
        action =
            MainMenuAction::Play;
    }

    if (ImGui::IsItemHovered())
    {
        mainMenuHoveredButton =
            0;
    }

    if (
        DrawMainMenuButton(
            "MainMenu_Editor",
            "ENTER EDITOR MODE",
            editorPosition,
            buttonSize,
            hoverFill,
            hoverBorder
        )
        )
    {
        action =
            MainMenuAction::Editor;
    }

    if (ImGui::IsItemHovered())
    {
        mainMenuHoveredButton =
            1;
    }

    
    if (
        DrawMainMenuButton(
            "MainMenu_Exit",
            "EXIT",
            exitPosition,
            buttonSize,
            hoverFill,
            hoverBorder
        )
        )
    {
        action =
            MainMenuAction::Exit;
    }

    if (ImGui::IsItemHovered())
    {
        mainMenuHoveredButton =
            2;
    }
    float linkButtonWidth =
        io.DisplaySize.x * 0.055f;

    float linkButtonHeight =
        io.DisplaySize.y * 0.085f;

    ImVec2 githubPosition =
        ImVec2(
            io.DisplaySize.x * 0.825f,
            io.DisplaySize.y * 0.842f
        );

    ImVec2 emailPosition =
        ImVec2(
            io.DisplaySize.x * 0.905f,
            io.DisplaySize.y * 0.842f
        );

    ImVec2 linkButtonSize =
        ImVec2(
            linkButtonWidth,
            linkButtonHeight
        );

    if (
        DrawMainMenuIconButton(
            "MainMenu_GitHub",
            githubPosition,
            linkButtonSize
        )
        )
    {
        OpenMainMenuLink(
            mainMenuGithubUrl
        );

        mainMenuLinkClicked =
            true;
    }

    if (
        DrawMainMenuIconButton(
            "MainMenu_Email",
            emailPosition,
            linkButtonSize
        )
        )
    {
        OpenMainMenuLink(
            mainMenuEmailUrl
        );

        mainMenuLinkClicked =
            true;
    }
    if (ImGui::IsItemHovered())
    {
        mainMenuHoveredButton =
            0;
    }
    ImGui::End();

    return action;
}
MoveAxis currentAxis = NONE;
GizmoMode currentGizmoMode = TRANSLATE;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        mouseClicked = true;
        isDragging = true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isDragging = false;
    }
}

SceneObject* selectedObject = nullptr;
// ================= UNDO / REDO TRANSFORM SYSTEM V1 =================

struct TransformHistoryState
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct TransformHistoryEntry
{
    SceneObject* object;
    std::string objectName;

    TransformHistoryState beforeState;
    TransformHistoryState afterState;
};

std::vector<TransformHistoryEntry> undoTransformStack;
std::vector<TransformHistoryEntry> redoTransformStack;

const int maxTransformHistoryCount =
80;

bool undoShortcutPressed =
false;

bool redoShortcutPressed =
false;

bool transformTrackingInitialized =
false;

SceneObject* transformTrackingObject =
nullptr;

TransformHistoryState transformTrackingState;

TransformHistoryState CaptureTransformHistoryState(
    SceneObject* object
)
{
    TransformHistoryState state;

    if (object == nullptr)
    {
        state.position =
            glm::vec3(0.0f);

        state.rotation =
            glm::vec3(0.0f);

        state.scale =
            glm::vec3(1.0f);

        return state;
    }

    state.position =
        object->transform.position;

    state.rotation =
        object->transform.rotation;

    state.scale =
        object->transform.scale;

    return state;
}

void ApplyTransformHistoryState(
    SceneObject* object,
    const TransformHistoryState& state
)
{
    if (object == nullptr)
        return;

    object->transform.position =
        state.position;

    object->transform.rotation =
        state.rotation;

    object->transform.scale =
        state.scale;
}

bool TransformHistoryStateChanged(
    const TransformHistoryState& a,
    const TransformHistoryState& b
)
{
    float positionDistance =
        glm::length(
            a.position -
            b.position
        );

    float rotationDistance =
        glm::length(
            a.rotation -
            b.rotation
        );

    float scaleDistance =
        glm::length(
            a.scale -
            b.scale
        );

    return
        positionDistance > 0.001f ||
        rotationDistance > 0.001f ||
        scaleDistance > 0.001f;
}

bool SceneStillContainsObject(
    Scene& scene,
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    for (SceneObject* sceneObject : scene.objects)
    {
        if (sceneObject == object)
            return true;
    }

    return false;
}

void ResetTransformHistoryTracking(
    SceneObject* selectedObject
)
{
    transformTrackingObject =
        selectedObject;

    transformTrackingInitialized =
        selectedObject != nullptr;

    if (selectedObject != nullptr)
    {
        transformTrackingState =
            CaptureTransformHistoryState(
                selectedObject
            );
    }
}

void PushTransformHistoryEntry(
    SceneObject* object,
    const TransformHistoryState& beforeState,
    const TransformHistoryState& afterState
)
{
    if (object == nullptr)
        return;

    if (
        !TransformHistoryStateChanged(
            beforeState,
            afterState
        )
        )
    {
        return;
    }

    TransformHistoryEntry entry;

    entry.object =
        object;

    entry.objectName =
        object->name;

    entry.beforeState =
        beforeState;

    entry.afterState =
        afterState;

    undoTransformStack.push_back(
        entry
    );

    redoTransformStack.clear();

    if (
        static_cast<int>(
            undoTransformStack.size()
            ) > maxTransformHistoryCount
        )
    {
        undoTransformStack.erase(
            undoTransformStack.begin()
        );
    }

    std::cout
        << "Undo history saved for: "
        << object->name
        << std::endl;
}

bool IsEditorTransformInputActive(
    GLFWwindow* window
)
{
    if (window == nullptr)
        return false;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        return true;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        return true;

    return false;
}

void UpdateTransformHistoryCapture(
    GLFWwindow* window,
    Scene& scene,
    SceneObject* selectedObject,
    bool isDragging
)
{
    if (selectedObject == nullptr)
    {
        transformTrackingInitialized =
            false;

        transformTrackingObject =
            nullptr;

        return;
    }

    if (
        !SceneStillContainsObject(
            scene,
            selectedObject
        )
        )
    {
        transformTrackingInitialized =
            false;

        transformTrackingObject =
            nullptr;

        return;
    }

    if (
        !transformTrackingInitialized ||
        transformTrackingObject != selectedObject
        )
    {
        ResetTransformHistoryTracking(
            selectedObject
        );

        return;
    }

    bool transformEditingActive =
        isDragging ||
        IsEditorTransformInputActive(
            window
        ) ||
        ImGui::IsAnyItemActive();

    if (transformEditingActive)
        return;

    TransformHistoryState currentState =
        CaptureTransformHistoryState(
            selectedObject
        );

    if (
        TransformHistoryStateChanged(
            transformTrackingState,
            currentState
        )
        )
    {
        PushTransformHistoryEntry(
            selectedObject,
            transformTrackingState,
            currentState
        );

        transformTrackingState =
            currentState;
    }
}

void UndoTransformChange(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    if (undoTransformStack.empty())
    {
        std::cout
            << "Undo stack is empty."
            << std::endl;

        return;
    }

    TransformHistoryEntry entry =
        undoTransformStack.back();

    undoTransformStack.pop_back();

    if (
        !SceneStillContainsObject(
            scene,
            entry.object
        )
        )
    {
        std::cout
            << "Undo skipped. Object no longer exists."
            << std::endl;

        return;
    }

    ApplyTransformHistoryState(
        entry.object,
        entry.beforeState
    );

    selectedObject =
        entry.object;

    redoTransformStack.push_back(
        entry
    );

    ResetTransformHistoryTracking(
        selectedObject
    );

    std::cout
        << "Undo transform: "
        << entry.objectName
        << std::endl;
}

void RedoTransformChange(
    Scene& scene,
    SceneObject*& selectedObject
)
{
    if (redoTransformStack.empty())
    {
        std::cout
            << "Redo stack is empty."
            << std::endl;

        return;
    }

    TransformHistoryEntry entry =
        redoTransformStack.back();

    redoTransformStack.pop_back();

    if (
        !SceneStillContainsObject(
            scene,
            entry.object
        )
        )
    {
        std::cout
            << "Redo skipped. Object no longer exists."
            << std::endl;

        return;
    }

    ApplyTransformHistoryState(
        entry.object,
        entry.afterState
    );

    selectedObject =
        entry.object;

    undoTransformStack.push_back(
        entry
    );

    ResetTransformHistoryTracking(
        selectedObject
    );

    std::cout
        << "Redo transform: "
        << entry.objectName
        << std::endl;
}

void HandleTransformHistoryShortcuts(
    GLFWwindow* window,
    Scene& scene,
    SceneObject*& selectedObject
)
{
    if (window == nullptr)
        return;

    if (ImGui::GetIO().WantTextInput)
        return;

    bool ctrlDown =
        glfwGetKey(
            window,
            GLFW_KEY_LEFT_CONTROL
        ) == GLFW_PRESS ||
        glfwGetKey(
            window,
            GLFW_KEY_RIGHT_CONTROL
        ) == GLFW_PRESS;

    bool zDown =
        glfwGetKey(
            window,
            GLFW_KEY_Z
        ) == GLFW_PRESS;

    bool yDown =
        glfwGetKey(
            window,
            GLFW_KEY_Y
        ) == GLFW_PRESS;

    if (
        ctrlDown &&
        zDown &&
        !undoShortcutPressed
        )
    {
        UndoTransformChange(
            scene,
            selectedObject
        );

        undoShortcutPressed =
            true;
    }

    if (!zDown)
    {
        undoShortcutPressed =
            false;
    }

    if (
        ctrlDown &&
        yDown &&
        !redoShortcutPressed
        )
    {
        RedoTransformChange(
            scene,
            selectedObject
        );

        redoShortcutPressed =
            true;
    }

    if (!yDown)
    {
        redoShortcutPressed =
            false;
    }
}
Light* selectedLight = nullptr;
Camera camera;
bool editorCameraStartFixed =false;
Frustum frustum;
PlayerController playerController;
ThirdPersonController thirdPersonController;
SceneObject* playerObject = nullptr;
glm::vec3 playerSpawnPosition =
glm::vec3(
    0.0f,
    0.0f,
    -5.0f
);
// ================= MOUSE CALLBACK ==================
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGuiIO& io =
        ImGui::GetIO();

    if (
        !rightMouseCameraActive ||
        io.WantCaptureMouse
        )
    {
        lastX =
            static_cast<float>(xpos);

        lastY =
            static_cast<float>(ypos);

        firstMouse =
            true;

        return;
    }

    if (
        firstMouse ||
        ignoreNextMouseDelta
        )
    {
        lastX =
            static_cast<float>(xpos);

        lastY =
            static_cast<float>(ypos);

        firstMouse =
            false;

        ignoreNextMouseDelta =
            false;

        return;
    }

    float xoffset =
        static_cast<float>(xpos) -
        lastX;

    float yoffset =
        lastY -
        static_cast<float>(ypos);

    lastX =
        static_cast<float>(xpos);

    lastY =
        static_cast<float>(ypos);

    if (
        std::abs(xoffset) > 80.0f ||
        std::abs(yoffset) > 80.0f
        )
    {
        return;
    }

    if (blockEditorMouseLook)
    {
        playCameraLookOffset +=
            yoffset *
            0.02f;

        playCameraLookOffset =
            glm::clamp(
                playCameraLookOffset,
                -0.6f,
                5.5f
            );

        return;
    }

    camera.ProcessMouseMovement(
        xoffset,
        yoffset
    );
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (selectedObject != nullptr)
    {
        float scaleSpeed = 0.2f;

        selectedObject->transform.scale +=
            glm::vec3(yoffset * scaleSpeed);

        selectedObject->transform.scale =
            glm::max(
                selectedObject->transform.scale,
                glm::vec3(0.1f)
            );
    }
}
// ================= COLLISION DEBUG VIEW =================

static std::vector<float> CreateCollisionCircleVertices(
    int segmentCount
)
{
    std::vector<float> vertices;

    const float pi =
        3.14159265359f;

    for (int i = 0; i < segmentCount; i++)
    {
        float angle =
            ((float)i / (float)segmentCount) *
            pi *
            2.0f;

        vertices.push_back(
            std::cos(angle)
        );

        vertices.push_back(
            0.0f
        );

        vertices.push_back(
            std::sin(angle)
        );
    }

    return vertices;
}

static bool CollisionDebugNameContains(
    const std::string& name,
    const std::string& token
)
{
    return
        name.find(token) != std::string::npos;
}

static bool ShouldDrawCollisionDebugForObject(
    SceneObject* object,
    SceneObject* playerObject
)
{
    if (object == nullptr)
        return false;

    if (!object->visible)
        return false;

    if (object == playerObject)
        return false;

    if (object->isCollider)
        return true;

    const std::string& name =
        object->name;

    if (CollisionDebugNameContains(name, "Coin"))
        return true;

    if (CollisionDebugNameContains(name, "Trigger"))
        return true;

    if (CollisionDebugNameContains(name, "Zone"))
        return true;

    if (CollisionDebugNameContains(name, "Monster"))
        return true;

    if (CollisionDebugNameContains(name, "Music"))
        return true;

    if (CollisionDebugNameContains(name, "Gate"))
        return true;

    if (CollisionDebugNameContains(name, "NPC"))
        return true;

    return false;
}

static float GetCollisionDebugRadius(
    SceneObject* object
)
{
    if (object == nullptr)
        return 0.0f;

    const std::string& name =
        object->name;

    if (object->isCollider)
    {
        return glm::max(
            object->colliderRadius,
            0.35f
        );
    }

    if (CollisionDebugNameContains(name, "Coin"))
        return 1.25f;

    if (
        CollisionDebugNameContains(name, "Trigger") ||
        CollisionDebugNameContains(name, "Zone")
        )
    {
        return 4.5f;
    }

    if (CollisionDebugNameContains(name, "Gate"))
        return 4.5f;

    if (CollisionDebugNameContains(name, "Monster"))
        return 2.4f;

    if (CollisionDebugNameContains(name, "NPC"))
        return 2.0f;

    if (CollisionDebugNameContains(name, "Music"))
        return 2.0f;

    return 0.0f;
}

static glm::vec3 GetCollisionDebugColor(
    SceneObject* object,
    SceneObject* selectedObject
)
{
    if (object == nullptr)
        return glm::vec3(1.0f);

    if (object == selectedObject)
        return glm::vec3(1.0f, 1.0f, 1.0f);

    const std::string& name =
        object->name;

    if (CollisionDebugNameContains(name, "Coin"))
        return glm::vec3(1.0f, 0.85f, 0.1f);

    if (
        CollisionDebugNameContains(name, "Trigger") ||
        CollisionDebugNameContains(name, "Zone")
        )
    {
        return glm::vec3(1.0f, 0.35f, 0.1f);
    }

    if (CollisionDebugNameContains(name, "Monster"))
        return glm::vec3(1.0f, 0.1f, 0.1f);

    if (
        CollisionDebugNameContains(name, "Music") ||
        CollisionDebugNameContains(name, "Gate") ||
        CollisionDebugNameContains(name, "NPC")
        )
    {
        return glm::vec3(0.3f, 0.6f, 1.0f);
    }

    return glm::vec3(0.1f, 1.0f, 0.25f);
}

static void DrawCollisionDebugRing(
    Shader& debugShader,
    unsigned int circleVAO,
    int circleVertexCount,
    const glm::vec3& position,
    float radius,
    const glm::vec3& color,
    const glm::mat4& view,
    const glm::mat4& projection,
    float yOffset
)
{
    if (circleVAO == 0)
        return;

    if (circleVertexCount <= 0)
        return;

    if (radius <= 0.0f)
        return;

    glm::mat4 ringModel =
        glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                position.x,
                position.y + yOffset,
                position.z
            )
        );

    ringModel =
        glm::scale(
            ringModel,
            glm::vec3(
                radius,
                1.0f,
                radius
            )
        );

    debugShader.setMat4(
        "model",
        glm::value_ptr(ringModel)
    );

    debugShader.setVec3(
        "axisColor",
        color
    );

    glDrawArrays(
        GL_LINE_LOOP,
        0,
        circleVertexCount
    );
}

static void DrawCollisionDebugView(
    Scene& scene,
    SceneObject* selectedObject,
    SceneObject* playerObject,
    Shader& debugShader,
    unsigned int circleVAO,
    int circleVertexCount,
    const glm::mat4& view,
    const glm::mat4& projection,
    float yOffset,
    float lineWidth
)
{
    glLineWidth(
        lineWidth
    );

    GLboolean depthTestEnabled =
        glIsEnabled(
            GL_DEPTH_TEST
        );

    glDisable(
        GL_DEPTH_TEST
    );

    debugShader.use();

    debugShader.setMat4(
        "view",
        glm::value_ptr(view)
    );

    debugShader.setMat4(
        "projection",
        glm::value_ptr(projection)
    );

    glBindVertexArray(
        circleVAO
    );

    if (playerObject != nullptr)
    {
        DrawCollisionDebugRing(
            debugShader,
            circleVAO,
            circleVertexCount,
            playerObject->transform.position,
            1.2f,
            glm::vec3(0.1f, 0.9f, 1.0f),
            view,
            projection,
            yOffset
        );
    }

    for (SceneObject* object : scene.objects)
    {
        if (
            !ShouldDrawCollisionDebugForObject(
                object,
                playerObject
            )
            )
        {
            continue;
        }

        float radius =
            GetCollisionDebugRadius(
                object
            );

        glm::vec3 color =
            GetCollisionDebugColor(
                object,
                selectedObject
            );

        DrawCollisionDebugRing(
            debugShader,
            circleVAO,
            circleVertexCount,
            object->transform.position,
            radius,
            color,
            view,
            projection,
            yOffset
        );
    }

    glBindVertexArray(
        0
    );

    if (depthTestEnabled)
    {
        glEnable(
            GL_DEPTH_TEST
        );
    }

    glLineWidth(
        1.0f
    );
}
float gizmoVertices[] =
{
    // X axis (red)
    0.0f, 0.0f, 0.0f,
    2.0f, 0.0f, 0.0f,

    // Y axis (green)
    0.0f, 0.0f, 0.0f,
    0.0f, 2.0f, 0.0f,

    // Z axis (blue)
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 2.0f
};
// ================= SHADERS =================
const char* gridVertexShader = R"(

#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
}

)";
const char* gridFragmentShader = R"(

#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.3, 0.3, 0.3, 1.0);
}

)";
const char* vertexShaderSource = R"(

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

)";
const char* fragmentShaderSource = R"(

#version 330 core
#define MAX_LIGHTS 12

out vec4 FragColor;
uniform vec3 sunDirection;
uniform vec3 sunColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform bool isSelected;
uniform sampler2D texture1;
uniform sampler2D terrainGrassTex;
uniform sampler2D terrainDirtTex;
uniform sampler2D terrainCliffTex;
uniform vec3 materialTint;
uniform vec3 viewPos;
uniform bool isProceduralTerrain;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform bool useTexture;
uniform bool useAtmosphereFog;
uniform vec3 atmosphereFogColor;
uniform float atmosphereFogStart;
uniform float atmosphereFogEnd;
uniform float atmosphereFogStrength;
void main()
{
    vec3 textureColor;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

if (isProceduralTerrain)
{
    float rawSlope =
        1.0 -
        clamp(norm.y, 0.0, 1.0);

    float slope =
        pow(
            rawSlope,
            0.55
        );

    vec2 grassUV =
        TexCoord * 3.0;

    vec2 dirtUV =
        TexCoord * 2.2;

    vec2 cliffUV =
        TexCoord * 2.0;

    vec3 grassTex =
        texture(
            terrainGrassTex,
            grassUV
        ).rgb;

    vec3 dirtTex =
        texture(
            terrainDirtTex,
            dirtUV
        ).rgb;

    vec3 cliffTex =
        texture(
            terrainCliffTex,
            cliffUV
        ).rgb;

    vec3 grassBase =
        vec3(
            0.32,
            0.42,
            0.20
        );

    vec3 dirtBase =
        vec3(
            0.52,
            0.39,
            0.24
        );

    vec3 cliffBase =
        vec3(
            0.45,
            0.43,
            0.39
        );

    vec3 grassColor =
        mix(
            grassBase,
            grassTex,
            0.45
        );

    vec3 dirtColor =
        mix(
            dirtBase,
            dirtTex,
            0.65
        );

    vec3 cliffColor =
        mix(
            cliffBase,
            cliffTex,
            0.75
        );

    float dirtBlend =
        smoothstep(
            0.08,
            0.22,
            slope
        );

    float cliffBlend =
        smoothstep(
            0.20,
            0.42,
            slope
        );

    float highMountainRock =
        smoothstep(
            12.0,
            28.0,
            FragPos.y
        ) * 0.35;

    float finalDirtBlend =
        clamp(
            dirtBlend,
            0.0,
            1.0
        );

    float finalCliffBlend =
        clamp(
            cliffBlend + highMountainRock,
            0.0,
            1.0
        );

    vec3 baseColor =
        mix(
            grassColor,
            dirtColor,
            finalDirtBlend
        );

    baseColor =
        mix(
            baseColor,
            cliffColor,
            finalCliffBlend
        );

    float broadVariation =
        sin(FragPos.x * 0.018) *
        cos(FragPos.z * 0.021);

    textureColor =
        baseColor +
        vec3(
            broadVariation * 0.020,
            broadVariation * 0.025,
            broadVariation * 0.010
        );
}
else if (useTexture)
{
    textureColor =
        texture(texture1, TexCoord).rgb *
        materialTint;
}
else
{
    textureColor =
        materialTint;
}
    vec3 result = vec3(0.0);
// Directional Light

vec3 dirLight =
normalize(-sunDirection);

float dirDiff =
max(dot(norm, dirLight), 0.12);

vec3 dirDiffuse =
dirDiff *
materialDiffuse *
textureColor *
sunColor;

vec3 dirReflect =
reflect(-dirLight,norm);

float dirSpec =
pow(
max(dot(viewDir,dirReflect),0.0),
materialShininess
);

vec3 dirSpecular =
materialSpecular *
dirSpec *
sunColor;

vec3 dirAmbient =
materialAmbient *
textureColor *
0.34;

result +=
    dirAmbient +
    dirDiffuse +
    dirSpecular;

float hemi =
    clamp(norm.y * 0.5 + 0.5, 0.0, 1.0);

vec3 hemiLight =
    mix(
        vec3(0.08, 0.07, 0.06),
        vec3(0.20, 0.24, 0.28),
        hemi
    );

result +=
    textureColor * hemiLight;

    for(int i = 0; i < MAX_LIGHTS; i++)
{
    if (length(lightColors[i]) < 0.001)
    {
        continue;
    }

    vec3 toLight =
        lightPositions[i] - FragPos;

    float distanceToLight =
        length(toLight);

    if (distanceToLight > 18.0)
    {
        continue;
    }

    vec3 lightDir =
        normalize(toLight);

    float attenuation =
        1.0 /
        (
            1.0 +
            0.18 * distanceToLight +
            0.08 * distanceToLight * distanceToLight
        );

    float diff =
        max(dot(norm, lightDir), 0.0);

    vec3 diffuse =
        diff *
        materialDiffuse *
        textureColor *
        lightColors[i] *
        attenuation;

    vec3 reflectDir =
        reflect(-lightDir, norm);

    float spec =
        pow(
            max(dot(viewDir, reflectDir), 0.0),
            materialShininess
        );

    vec3 specular =
        materialSpecular *
        spec *
        lightColors[i] *
        attenuation;

    vec3 ambient =
        materialAmbient *
        textureColor *
        lightColors[i] *
        0.02 *
        attenuation;

    result +=
        ambient +
        diffuse +
        specular;
}
if(isSelected)
{
    result *= 1.3;
}


result *= 0.90;

if (useAtmosphereFog)
{
    float fogDistance =
        distance(
            viewPos,
            FragPos
        );

    float fogRange =
        max(
            atmosphereFogEnd -
            atmosphereFogStart,
            0.001
        );

    float fogAmount =
        clamp(
            (fogDistance - atmosphereFogStart) /
            fogRange,
            0.0,
            1.0
        );

    fogAmount =
        clamp(
            fogAmount *
            atmosphereFogStrength,
            0.0,
            1.0
        );

    result =
        mix(
            result,
            atmosphereFogColor,
            fogAmount
        );
}
result =
    clamp(
        result,
        0.0,
        1.0
    );

FragColor =
    vec4(
        result,
        1.0
    );

}

)";
const char* lightVertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* lightFragmentSource = R"(

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0);
}

)";
const char* skyboxVertex = R"(

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aPos;

    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}

)";
const char* skyboxFragment = R"(

#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}

)";
const char* gizmoVertexShader = R"(

#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

)";
const char* gizmoFragmentShader = R"(

#version 330 core

out vec4 FragColor;

uniform vec3 axisColor;

void main()
{
    FragColor = vec4(axisColor, 1.0);
}

)";
std::vector<std::string> faces =
{
    "textures/skybox/right.jpg",
    "textures/skybox/left.jpg",
    "textures/skybox/top.jpg",
    "textures/skybox/bottom.jpg",
    "textures/skybox/front.jpg",
    "textures/skybox/back.jpg"
};
float GetTerrainHeight(
    float x,
    float z
)
{
    float dist =
        std::sqrt(x * x + z * z);

    float base =
        std::sin(x * 0.010f) * 2.2f +
        std::cos(z * 0.012f) * 1.8f +
        std::sin((x + z) * 0.008f) * 1.5f;

    base +=
        std::sin(x * 0.035f) *
        std::cos(z * 0.030f) *
        2.2f;

    float peak1 =
        26.0f *
        std::exp(
            -(
                (x - 55.0f) * (x - 55.0f) +
                (z + 35.0f) * (z + 35.0f)
                ) / 2200.0f
        );

    float peak2 =
        18.0f *
        std::exp(
            -(
                (x + 70.0f) * (x + 70.0f) +
                (z - 45.0f) * (z - 45.0f)
                ) / 2600.0f
        );

    float peak3 =
        22.0f *
        std::exp(
            -(
                (x - 15.0f) * (x - 15.0f) +
                (z - 95.0f) * (z - 95.0f)
                ) / 1800.0f
        );

    float ring =
        0.0f;

    if (dist > 120.0f)
    {
        float t =
            (dist - 120.0f) / 90.0f;

        t =
            glm::clamp(
                t,
                0.0f,
                1.0f
            );

        ring =
            t * t * 38.0f;
    }

    return
        base +
        peak1 +
        peak2 +
        peak3 +
        ring -
        2.0f;
}
float GetPlayerTerrainY(
    float x,
    float z
)
{
    return
        GetTerrainHeight(
            x,
            z
        ) + 0.55f;
}

glm::vec3 GetTerrainNormal(
    float x,
    float z
)
{
    float heightLeft =
        GetTerrainHeight(
            x - 1.0f,
            z
        );

    float heightRight =
        GetTerrainHeight(
            x + 1.0f,
            z
        );

    float heightDown =
        GetTerrainHeight(
            x,
            z - 1.0f
        );

    float heightUp =
        GetTerrainHeight(
            x,
            z + 1.0f
        );

    glm::vec3 normal =
        glm::normalize(
            glm::vec3(
                heightLeft - heightRight,
                4.0f,
                heightDown - heightUp
            )
        );

    return normal;
}

float GetObjectTerrainY(
    float x,
    float z,
    float offset = 0.05f
)
{
    return
        GetTerrainHeight(
            x,
            z
        ) + offset;
}

glm::vec3 SnapObjectToTerrain(
    glm::vec3 position,
    float offset = 0.05f
)
{
    position.y =
        GetObjectTerrainY(
            position.x,
            position.z,
            offset
        );

    return position;
}

bool IsGoodTerrainSpawnPoint(
    float x,
    float z
)
{
    glm::vec3 normal =
        GetTerrainNormal(
            x,
            z
        );

    return normal.y > 0.72f;
}
void SetObjectMetadata(
    SceneObject* object,
    const std::string& assetId,
    AssetType assetType,
    SpawnSource spawnSource,
    bool persistent,
    bool showInHierarchy
)
{
    if (object == nullptr)
        return;

    object->assetId =
        assetId;

    object->assetType =
        assetType;

    object->spawnSource =
        spawnSource;

    object->persistent =
        persistent;

    object->showInHierarchy =
        showInHierarchy;
}
void AddTerrainVertex(
    std::vector<float>& vertices,
    float x,
    float z
)
{
    float y =
        GetTerrainHeight(
            x,
            z
        );

    glm::vec3 normal =
        GetTerrainNormal(
            x,
            z
        );
    float textureU =
        x * 0.035f;

    float textureV =
        z * 0.035f;

    // position
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);

    // normal
    vertices.push_back(normal.x);
    vertices.push_back(normal.y);
    vertices.push_back(normal.z);

    // texcoord
    vertices.push_back(textureU);
    vertices.push_back(textureV);
}

void BuildProceduralTerrain(
    std::vector<float>& vertices,
    float terrainSize,
    int resolution
)
{
    vertices.clear();

    float halfSize =
        terrainSize * 0.5f;

    float step =
        terrainSize /
        static_cast<float>(resolution);

    for (int x = 0; x < resolution; x++)
    {
        for (int z = 0; z < resolution; z++)
        {
            float x0 =
                -halfSize +
                x * step;

            float z0 =
                -halfSize +
                z * step;

            float x1 =
                x0 + step;

            float z1 =
                z0 + step;

            // first triangle
            AddTerrainVertex(vertices, x0, z0);
            AddTerrainVertex(vertices, x1, z0);
            AddTerrainVertex(vertices, x1, z1);

            // second triangle
            AddTerrainVertex(vertices, x0, z0);
            AddTerrainVertex(vertices, x1, z1);
            AddTerrainVertex(vertices, x0, z1);
        }
    }
}
void UpdatePlayModeCameraFollow(
    Camera& camera,
    SceneObject* playerObject,
    float deltaTime
)
{
    if (playerObject == nullptr)
        return;

    static bool cameraFollowInitialized =
        false;

    static glm::vec3 smoothedFollowPosition =
        glm::vec3(
            0.0f
        );

    static glm::vec3 smoothedCameraPosition =
        glm::vec3(
            0.0f
        );

    static glm::vec3 smoothedLookTarget =
        glm::vec3(
            0.0f
        );

    static glm::vec3 smoothedCameraForward =
        glm::vec3(
            0.0f,
            0.0f,
            -1.0f
        );

    glm::vec3 playerPosition =
        playerObject->transform.position;

    // Follow the player's ground position, not jump height.
    // This prevents the camera from shaking up/down during jumping.
    glm::vec3 playerGroundPosition =
        glm::vec3(
            playerPosition.x,
            GetPlayerTerrainY(
                playerPosition.x,
                playerPosition.z
            ),
            playerPosition.z
        );

    glm::vec3 targetCameraForward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(targetCameraForward) < 0.001f)
    {
        targetCameraForward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    targetCameraForward =
        glm::normalize(
            targetCameraForward
        );

    float cameraDistance =
        6.0f;

    float cameraHeight =
        3.0f +
        playCameraLookOffset *
        0.25f;

    float lookHeight =
        1.45f +
        playCameraLookOffset;

    glm::vec3 targetLookPosition =
        playerGroundPosition +
        glm::vec3(
            0.0f,
            lookHeight,
            0.0f
        );

    glm::vec3 targetCameraPosition =
        playerGroundPosition -
        targetCameraForward *
        cameraDistance +
        glm::vec3(
            0.0f,
            cameraHeight,
            0.0f
        );

    if (
        !cameraFollowInitialized ||
        deltaTime > 0.2f
        )
    {
        smoothedFollowPosition =
            playerGroundPosition;

        smoothedCameraPosition =
            targetCameraPosition;

        smoothedLookTarget =
            targetLookPosition;

        smoothedCameraForward =
            targetCameraForward;

        camera.Position =
            targetCameraPosition;

        camera.Front =
            glm::normalize(
                targetLookPosition -
                camera.Position
            );

        cameraFollowInitialized =
            true;

        return;
    }

    float followSmooth =
        1.0f -
        std::exp(
            -10.0f *
            deltaTime
        );

    float cameraPositionSmooth =
        1.0f -
        std::exp(
            -7.0f *
            deltaTime
        );

    float cameraDirectionSmooth =
        1.0f -
        std::exp(
            -9.0f *
            deltaTime
        );

    smoothedFollowPosition =
        glm::mix(
            smoothedFollowPosition,
            playerGroundPosition,
            followSmooth
        );

    smoothedCameraForward =
        glm::normalize(
            glm::mix(
                smoothedCameraForward,
                targetCameraForward,
                cameraDirectionSmooth
            )
        );

    targetLookPosition =
        smoothedFollowPosition +
        glm::vec3(
            0.0f,
            lookHeight,
            0.0f
        );

    targetCameraPosition =
        smoothedFollowPosition -
        smoothedCameraForward *
        cameraDistance +
        glm::vec3(
            0.0f,
            cameraHeight,
            0.0f
        );

    smoothedCameraPosition =
        glm::mix(
            smoothedCameraPosition,
            targetCameraPosition,
            cameraPositionSmooth
        );

    smoothedLookTarget =
        glm::mix(
            smoothedLookTarget,
            targetLookPosition,
            followSmooth
        );

    camera.Position =
        smoothedCameraPosition;

    camera.Front =
        glm::normalize(
            smoothedLookTarget -
            camera.Position
        );
}
bool ObjectNameContains(
    SceneObject* object,
    const std::string& text
)
{
    if (object == nullptr)
        return false;

    return
        object->name.find(text) != std::string::npos;
}
bool IsEditorSelectableObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (!object->visible)
        return false;

    if (object->name == "Player")
        return false;

    if (object->name == "Ground")
        return false;

    if (object->name == "Procedural Terrain")
        return false;

    if (object->name.find("Generated") != std::string::npos)
        return false;

    if (object->name.find("Border") != std::string::npos)
        return false;

    if (object->name.find("Grass Terrain Tile") != std::string::npos)
        return false;

    return true;
}

float GetEditorSelectionRadius(
    SceneObject* object
)
{
    if (object == nullptr)
        return 1.0f;

    if (ObjectNameContains(object, "House"))
        return 8.0f;

    if (ObjectNameContains(object, "Campfire"))
        return 3.0f;

    if (ObjectNameContains(object, "Torch"))
        return 2.5f;

    if (ObjectNameContains(object, "Tree"))
        return 4.5f;

    if (ObjectNameContains(object, "Rock"))
        return 3.0f;

    if (ObjectNameContains(object, "Bush"))
        return 2.5f;

    if (ObjectNameContains(object, "Wood Log"))
        return 2.5f;

    if (ObjectNameContains(object, "Tree Stump"))
        return 2.5f;

    if (ObjectNameContains(object, "Wall"))
        return 4.0f;

    if (ObjectNameContains(object, "Fence"))
        return 3.0f;

    if (ObjectNameContains(object, "Platform"))
        return 4.0f;

    if (ObjectNameContains(object, "Path Tile"))
        return 3.5f;

    float maxScale =
        glm::max(
            glm::max(
                object->transform.scale.x,
                object->transform.scale.y
            ),
            object->transform.scale.z
        );

    return glm::clamp(
        maxScale * 1.2f,
        1.2f,
        6.0f
    );
}

void SelectEditorObject(
    SceneObject*& selectedObject,
    SceneObject* newSelection
)
{
    if (selectedObject != nullptr)
        selectedObject->isSelected =
        false;

    selectedObject =
        newSelection;

    if (selectedObject != nullptr)
    {
        selectedObject->isSelected =
            true;

        std::cout
            << "Selected object: "
            << selectedObject->name
            << std::endl;
    }
}
bool IsCampfireObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    return ObjectNameContains(
        object,
        "Campfire"
    );
}
bool IsImportantRenderObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return true;

    if (object == selectedObject)
        return true;

    if (object == nearbyInteractableObject)
        return true;

    if (object->name == "Player")
        return true;

    if (object->name == "Procedural Terrain")
        return true;

    if (ObjectNameContains(object, "House"))
        return true;

    if (ObjectNameContains(object, "Torch"))
        return true;

    if (object->attachedLight != nullptr)
        return true;

    if (ObjectNameContains(object, "Camp House"))
        return true;

    return false;
}

bool IsTinyPerformanceProp(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (ObjectNameContains(object, "Grass"))
        return true;

    if (ObjectNameContains(object, "Plant"))
        return true;

    if (ObjectNameContains(object, "Flower"))
        return true;

    if (ObjectNameContains(object, "Wheat"))
        return true;

    return false;
}

bool IsSmallPerformanceProp(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (ObjectNameContains(object, "Bush"))
        return true;

    if (ObjectNameContains(object, "Tree Stump"))
        return true;

    if (ObjectNameContains(object, "Wood Log"))
        return true;

    if (ObjectNameContains(object, "Camp Prop"))
        return true;

    return false;
}

bool IsRockPerformanceProp(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (ObjectNameContains(object, "Rock"))
        return true;

    return false;
}

bool IsMediumPerformanceProp(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (ObjectNameContains(object, "Pine Tree"))
        return true;

    if (ObjectNameContains(object, "Common Tree"))
        return true;

    if (ObjectNameContains(object, "Birch"))
        return true;

    if (ObjectNameContains(object, "Willow"))
        return true;

    if (ObjectNameContains(object, "Camp Tree"))
        return true;

    if (ObjectNameContains(object, "Tree"))
        return true;

    return false;
}
bool IsCoinObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorGameplayType == "Coin")
        return true;

    if (object->name.find("Coin") != std::string::npos)
        return true;

    return false;
}
SceneObject* FindPlayerObject(
    Scene& scene
);
bool IsTriggerZoneObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorGameplayType == "TriggerZone")
        return true;

    if (object->name.find("Trigger Zone") != std::string::npos)
        return true;

    return false;
}
bool IsMonsterSpawnObject(
    SceneObject* object
);

bool IsMusicGateObject(
    SceneObject* object
);

bool IsMusicNpcObject(
    SceneObject* object
);
bool WorldToScreenPosition(
    const glm::vec3& worldPosition,
    const glm::mat4& view,
    const glm::mat4& projection,
    int screenWidth,
    int screenHeight,
    ImVec2& screenPosition
)
{
    glm::vec4 clipPosition =
        projection *
        view *
        glm::vec4(
            worldPosition,
            1.0f
        );

    if (clipPosition.w <= 0.01f)
        return false;

    glm::vec3 ndc =
        glm::vec3(
            clipPosition
        ) /
        clipPosition.w;

    if (
        ndc.x < -1.2f ||
        ndc.x > 1.2f ||
        ndc.y < -1.2f ||
        ndc.y > 1.2f
        )
    {
        return false;
    }

    screenPosition.x =
        (
            ndc.x * 0.5f +
            0.5f
            ) *
        static_cast<float>(
            screenWidth
            );

    screenPosition.y =
        (
            1.0f -
            (
                ndc.y * 0.5f +
                0.5f
                )
            ) *
        static_cast<float>(
            screenHeight
            );

    return true;
}
bool IsMonsterSpawnObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorGameplayType == "MonsterSpawn")
        return true;

    if (object->name.find("Monster Spawn") != std::string::npos)
        return true;

    return false;
}
SceneObject* FindMonsterSpawnObject(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (IsMonsterSpawnObject(object))
            return object;
    }

    return nullptr;
}
bool IsMusicGateObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorGameplayType == "MusicGate")
        return true;

    if (object->name == "Music Gate")
        return true;

    return false;
}

bool IsMusicNpcObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->editorGameplayType == "MusicNPC")
        return true;

    if (object->name.find("Music NPC") != std::string::npos)
        return true;

    return false;
}

SceneObject* FindMusicNpcObject(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (IsMusicNpcObject(object))
            return object;
    }

    return nullptr;
}

void ResetMusicRescueForPlay(
    Scene& scene
)
{
    musicRescueActive =
        false;

    musicNpcChasingMonster =
        false;

    musicRescueSoundStarted =
        false;

    monsterDefeatedByMusic =
        false;

    musicRescueWin =
        false;

    musicRescueText =
        "Music Rescue: Reach the Music Gate while escaping.";

    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (IsMusicNpcObject(object))
        {
            object->visible =
                false;
        }

        if (IsMusicGateObject(object))
        {
            object->visible =
                true;
        }
    }
}

void RestoreMusicRescueForEditor(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (IsMusicNpcObject(object))
        {
            object->visible =
                true;
        }

        if (IsMusicGateObject(object))
        {
            object->visible =
                true;
        }
    }
}

void ActivateMusicRescueEvent(
    Scene& scene,
    AudioSystem& audioSystem
)
{
    musicRescueActive =
        true;

    musicNpcChasingMonster =
        true;

    musicRescueText =
        "Music Rescue: Music NPC activated!";

    SceneObject* musicNpc =
        FindMusicNpcObject(
            scene
        );

    if (musicNpc != nullptr)
    {
        musicNpc->visible =
            true;
    }

    if (!musicRescueSoundStarted)
    {
        audioSystem.PlayFromStart(
            "music_rescue",
            0.90f
        );

        musicRescueSoundStarted =
            true;
    }

    std::cout
        << "Music rescue event activated."
        << std::endl;
}

void UpdateMusicRescueEvent(
    Scene& scene,
    AudioSystem& audioSystem,
    float deltaTime
)
{
    if (monsterEscapeGameMode.playerCaught)
        return;

    if (musicRescueWin)
        return;

    SceneObject* player =
        FindPlayerObject(
            scene
        );

    SceneObject* monster =
        FindMonsterSpawnObject(
            scene
        );

    SceneObject* musicNpc =
        FindMusicNpcObject(
            scene
        );

    if (player == nullptr)
        return;

    if (monster == nullptr)
        return;

    if (musicNpc == nullptr)
    {
        musicRescueText =
            "Music Rescue: Place a Music NPC in the editor.";

        return;
    }

    if (!monsterEscapeGameMode.active)
    {
        musicRescueText =
            "Music Rescue: Waiting for monster chase.";

        return;
    }

    if (!musicRescueActive)
    {
        for (SceneObject* object : scene.objects)
        {
            if (!IsMusicGateObject(object))
                continue;

            glm::vec3 playerPosition =
                player->transform.position;

            glm::vec3 gatePosition =
                object->transform.position;

            float distance =
                glm::distance(
                    glm::vec3(
                        playerPosition.x,
                        0.0f,
                        playerPosition.z
                    ),
                    glm::vec3(
                        gatePosition.x,
                        0.0f,
                        gatePosition.z
                    )
                );

            float finalGateRadius =
                glm::max(
                    musicGateRadius,
                    glm::max(
                        object->transform.scale.x,
                        object->transform.scale.z
                    ) * 0.75f
                );

            if (distance <= finalGateRadius)
            {
                ActivateMusicRescueEvent(
                    scene,
                    audioSystem
                );

                break;
            }
        }
    }

    if (!musicNpcChasingMonster)
        return;

    if (!monster->visible)
        return;

    musicNpc->visible =
        true;

    glm::vec3 npcPosition =
        musicNpc->transform.position;

    glm::vec3 monsterPosition =
        monster->transform.position;

    glm::vec3 direction =
        glm::vec3(
            monsterPosition.x - npcPosition.x,
            0.0f,
            monsterPosition.z - npcPosition.z
        );

    float distanceToMonster =
        glm::length(
            direction
        );

    if (distanceToMonster <= musicNpcCatchRadius)
    {
        monster->visible =
            false;

        monsterDefeatedByMusic =
            true;

        musicRescueWin =
            true;

        musicNpcChasingMonster =
            false;

        monsterEscapeGameMode.active =
            false;

        monsterEscapeGameMode.eventText =
            "Monster Event: WIN - Monster defeated!";

        musicRescueText =
            "Music Rescue: WIN - Monster defeated by Music NPC!";

        audioSystem.Stop(
            "monster_chase"
        );

        audioSystem.Stop(
            "music_rescue"
        );

        audioSystem.PlayFromStart(
            "coin_win",
            1.0f
        );

        std::cout
            << "Music rescue win. Monster defeated."
            << std::endl;

        return;
    }

    if (distanceToMonster < 0.001f)
        return;

    direction =
        glm::normalize(
            direction
        );

    musicNpc->transform.position +=
        direction *
        musicNpcSpeed *
        deltaTime;

    musicNpc->transform.position.y =
        GetTerrainHeight(
            musicNpc->transform.position.x,
            musicNpc->transform.position.z
        ) +
        musicNpcTerrainOffset;

    musicNpc->transform.rotation =
        glm::vec3(
            90.0f,
            180.0f,
            0.0f
        );

    musicNpc->transform.rotation.z =
        0.0f;

    musicRescueText =
        "Music Rescue: Music NPC is chasing the monster!";
}
void DrawRuntimeResultOverlay()
{
    if (!ShouldLockPlayerAfterRuntimeResult())
    {
        return;
    }

    ImGuiIO& io =
        ImGui::GetIO();

    ImGui::SetNextWindowPos(
        ImVec2(
            io.DisplaySize.x * 0.5f - 260.0f,
            io.DisplaySize.y * 0.5f - 95.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            520.0f,
            190.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Runtime Result Overlay",
        nullptr,
        flags
    );

    ImGui::Spacing();
    ImGui::Spacing();

    if (musicRescueWin)
    {
        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "              YOU WIN"
        );

        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::Separator();

        ImGui::Text(
            "Monster defeated by Music NPC."
        );

        ImGui::Text(
            "The rescue event was completed successfully."
        );
    }
    else if (coinRushGameMode.won)
    {
        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "        COIN HUNT COMPLETE"
        );

        ImGui::TextColored(
            ImVec4(
                0.2f,
                1.0f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::Separator();

        ImGui::Text(
            "All coins were collected before time ended."
        );

        ImGui::Text(
            "Coin objective completed successfully."
        );
    }
    else if (coinRushGameMode.lost)
    {
        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "          TIME IS OVER"
        );

        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::Separator();

        ImGui::Text(
            "The player did not collect all coins in time."
        );

        ImGui::Text(
            "Try collecting the coins faster."
        );
    }
    else if (monsterEscapeGameMode.playerCaught)
    {
        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "             YOU LOSE"
        );

        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.2f,
                0.2f,
                1.0f
            ),
            "=============================="
        );

        ImGui::Separator();

        ImGui::Text(
            "Monster caught the player."
        );

        ImGui::Text(
            "Try reaching the Music Gate faster."
        );
    }

    ImGui::Separator();

    ImGui::Text(
        "Press Stop to return to Editor Mode."
    );

    ImGui::End();
}
SceneObject* FindPlayerObject(
    Scene& scene
)
{
    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (object->name == "Player")
            return object;
    }

    return nullptr;
}
enum class SceneHealthLevel
{
    Info,
    Warning,
    Error
};

struct SceneHealthEntry
{
    SceneHealthLevel level;
    std::string message;
};

void AddSceneHealthEntry(
    std::vector<SceneHealthEntry>& entries,
    SceneHealthLevel level,
    const std::string& message
)
{
    SceneHealthEntry entry;

    entry.level =
        level;

    entry.message =
        message;

    entries.push_back(
        entry
    );
}

ImVec4 GetSceneHealthColor(
    SceneHealthLevel level
)
{
    if (level == SceneHealthLevel::Error)
    {
        return ImVec4(
            1.0f,
            0.25f,
            0.25f,
            1.0f
        );
    }

    if (level == SceneHealthLevel::Warning)
    {
        return ImVec4(
            1.0f,
            0.75f,
            0.20f,
            1.0f
        );
    }

    return ImVec4(
        0.35f,
        1.0f,
        0.45f,
        1.0f
    );
}

const char* GetSceneHealthPrefix(
    SceneHealthLevel level
)
{
    if (level == SceneHealthLevel::Error)
        return "[ERROR]";

    if (level == SceneHealthLevel::Warning)
        return "[WARNING]";

    return "[OK]";
}

std::vector<SceneHealthEntry> EvaluateSceneHealth(
    Scene& scene,
    SceneObject* playerObject,
    bool useAnimatedPlayerVisual
)
{
    std::vector<SceneHealthEntry> entries;

    int totalObjects =
        0;

    int visibleObjects =
        0;

    int hiddenObjects =
        0;

    int coinCount =
        0;

    int triggerZoneCount =
        0;

    int monsterSpawnCount =
        0;

    int musicGateCount =
        0;

    int musicNpcCount =
        0;

    int objectsBelowTerrain =
        0;

    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        totalObjects++;

        if (object->visible)
        {
            visibleObjects++;
        }
        else
        {
            hiddenObjects++;
        }

        if (IsCoinObject(object))
        {
            coinCount++;
        }

        if (IsTriggerZoneObject(object))
        {
            triggerZoneCount++;
        }

        if (IsMonsterSpawnObject(object))
        {
            monsterSpawnCount++;
        }

        if (IsMusicGateObject(object))
        {
            musicGateCount++;
        }

        if (IsMusicNpcObject(object))
        {
            musicNpcCount++;
        }

        bool skipTerrainCheck =
            object->name == "Player" ||
            object->name == "Procedural Terrain" ||
            object->name == "Ground" ||
            object->name.find("Generated") != std::string::npos ||
            object->name.find("Grass Terrain Tile") != std::string::npos;

        if (
            object->visible &&
            !skipTerrainCheck
            )
        {
            float terrainY =
                GetTerrainHeight(
                    object->transform.position.x,
                    object->transform.position.z
                );

            if (object->transform.position.y < terrainY - 2.0f)
            {
                objectsBelowTerrain++;
            }
        }
    }

    int totalLights =
        0;

    int pointLights =
        0;

    for (Light* light : scene.lights)
    {
        if (light == nullptr)
            continue;

        totalLights++;

        if (light->type != LightType::Directional)
        {
            pointLights++;
        }
    }

    SceneObject* foundPlayer =
        playerObject;

    if (foundPlayer == nullptr)
    {
        foundPlayer =
            FindPlayerObject(
                scene
            );
    }

    if (foundPlayer == nullptr)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Error,
            "No Player object was found. Play Mode cannot be tested correctly."
        );
    }
    else
    {
        float expectedPlayerY =
            GetPlayerTerrainY(
                foundPlayer->transform.position.x,
                foundPlayer->transform.position.z
            );

        if (foundPlayer->transform.position.y < expectedPlayerY - 1.0f)
        {
            AddSceneHealthEntry(
                entries,
                SceneHealthLevel::Error,
                "Player is below the terrain. Use Player Tools to respawn or place the player on terrain."
            );
        }
        else
        {
            AddSceneHealthEntry(
                entries,
                SceneHealthLevel::Info,
                "Player exists and is positioned above the terrain."
            );
        }
    }

    if (totalObjects == 0)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Error,
            "The scene has no objects."
        );
    }
    else
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "Scene contains " +
            std::to_string(totalObjects) +
            " objects, including " +
            std::to_string(visibleObjects) +
            " visible objects."
        );
    }

    if (visibleObjects == 0)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Error,
            "All objects are hidden. The scene will appear empty in Play Mode."
        );
    }

    bool hasCoinObjective =
        coinCount > 0;

    bool hasMonsterObjective =
        triggerZoneCount > 0 &&
        monsterSpawnCount > 0;

    bool hasMusicObjective =
        musicGateCount > 0 &&
        musicNpcCount > 0;

    bool hasAnyObjective =
        hasCoinObjective ||
        hasMonsterObjective ||
        hasMusicObjective;

    if (!hasAnyObjective)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "No clear gameplay objective was found. Add coins, a trigger zone with monster spawn, or a music gate objective."
        );
    }
    else
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "At least one playable objective was found."
        );
    }

    if (coinCount > 0)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "Coin Hunt objective detected with " +
            std::to_string(coinCount) +
            " coins."
        );
    }

    if (
        triggerZoneCount > 0 &&
        monsterSpawnCount == 0
        )
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "Trigger zone exists, but no monster spawn object was found."
        );
    }

    if (
        monsterSpawnCount > 0 &&
        triggerZoneCount == 0
        )
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "Monster spawn exists, but no trigger zone was found."
        );
    }

    if (
        musicGateCount > 0 &&
        musicNpcCount == 0
        )
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "Music Gate exists, but no Music NPC was found."
        );
    }

    if (
        musicNpcCount > 0 &&
        musicGateCount == 0
        )
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "Music NPC exists, but no Music Gate was found."
        );
    }

    if (objectsBelowTerrain > 0)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            std::to_string(objectsBelowTerrain) +
            " visible objects appear to be below the terrain."
        );
    }

    if (totalObjects > 1200)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "The scene contains many objects. Performance may decrease on weaker hardware."
        );
    }

    if (pointLights > 12)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Error,
            "More than 12 point lights were found. The shader supports only 12 active point lights."
        );
    }
    else if (pointLights > 10)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Warning,
            "The scene is close to the 12 point light limit."
        );
    }
    else
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "Lighting setup is within the supported limit."
        );
    }

    if (useAnimatedPlayerVisual)
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "Animated player visual mode is enabled."
        );
    }
    else
    {
        AddSceneHealthEntry(
            entries,
            SceneHealthLevel::Info,
            "Classic player visual mode is enabled."
        );
    }

    return entries;
}

void DrawSceneHealthValidator(
    Scene& scene,
    SceneObject* playerObject,
    bool useAnimatedPlayerVisual
)
{
    ImGui::SetNextWindowPos(
        ImVec2(
            centerX + 520.0f,
            topY
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            440.0f,
            460.0f
        ),
        ImGuiCond_FirstUseEver
    );

    ImGui::Begin(
        "Playability Validator"
    );

    ImGui::Text(
        "Live Edit-Test Scene Check"
    );

    ImGui::TextDisabled(
        "Checks if the current scene is ready for Play Mode."
    );

    ImGui::Separator();

    std::vector<SceneHealthEntry> entries =
        EvaluateSceneHealth(
            scene,
            playerObject,
            useAnimatedPlayerVisual
        );

    int coinCount =
        0;

    int triggerZoneCount =
        0;

    int monsterSpawnCount =
        0;

    int musicGateCount =
        0;

    int musicNpcCount =
        0;

    int colliderCount =
        0;

    int visibleObjectCount =
        0;

    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (object->visible)
        {
            visibleObjectCount++;
        }

        if (object->isCollider)
        {
            colliderCount++;
        }

        if (IsCoinObject(object))
        {
            coinCount++;
        }

        if (IsTriggerZoneObject(object))
        {
            triggerZoneCount++;
        }

        if (IsMonsterSpawnObject(object))
        {
            monsterSpawnCount++;
        }

        if (IsMusicGateObject(object))
        {
            musicGateCount++;
        }

        if (IsMusicNpcObject(object))
        {
            musicNpcCount++;
        }
    }

    bool hasPlayer =
        playerObject != nullptr ||
        FindPlayerObject(scene) != nullptr;

    bool hasCoinObjective =
        coinCount > 0;

    bool hasMonsterObjective =
        triggerZoneCount > 0 &&
        monsterSpawnCount > 0;

    bool hasMusicObjective =
        musicGateCount > 0 &&
        musicNpcCount > 0;

    bool scenePlayable =
        hasPlayer &&
        (
            hasCoinObjective ||
            hasMonsterObjective ||
            hasMusicObjective
            );

    auto DrawStatusLine =
        [](
            const char* label,
            bool ok,
            const char* detail
            )
        {
            ImGui::TextColored(
                ok
                ? ImVec4(0.35f, 1.0f, 0.45f, 1.0f)
                : ImVec4(1.0f, 0.35f, 0.25f, 1.0f),
                "%s",
                ok ? "[OK]" : "[MISS]"
            );

            ImGui::SameLine();

            ImGui::Text(
                "%s",
                label
            );

            ImGui::SameLine();

            ImGui::TextDisabled(
                "%s",
                detail
            );
        };

    ImGui::Text(
        "Gameplay Readiness"
    );

    DrawStatusLine(
        "Player",
        hasPlayer,
        hasPlayer ? "ready" : "missing"
    );

    DrawStatusLine(
        "Coin Objective",
        hasCoinObjective,
        hasCoinObjective ? "coins found" : "no coins"
    );

    DrawStatusLine(
        "Monster Objective",
        hasMonsterObjective,
        hasMonsterObjective ? "trigger + spawn" : "needs trigger + spawn"
    );

    DrawStatusLine(
        "Music Objective",
        hasMusicObjective,
        hasMusicObjective ? "gate + NPC" : "needs gate + NPC"
    );

    ImGui::Separator();

    if (scenePlayable)
    {
        ImGui::TextColored(
            ImVec4(
                0.35f,
                1.0f,
                0.45f,
                1.0f
            ),
            "SCENE STATUS: PLAYABLE"
        );
    }
    else
    {
        ImGui::TextColored(
            ImVec4(
                1.0f,
                0.35f,
                0.25f,
                1.0f
            ),
            "SCENE STATUS: NOT PLAYABLE"
        );
    }

    ImGui::TextDisabled(
        "Fix missing gameplay objects, then press TAB to test."
    );

    ImGui::Separator();

    ImGui::Text(
        "Scene Counts"
    );

    ImGui::Text(
        "Visible Objects: %d",
        visibleObjectCount
    );

    ImGui::Text(
        "Colliders: %d",
        colliderCount
    );

    ImGui::Text(
        "Coins: %d",
        coinCount
    );

    ImGui::Text(
        "Trigger Zones: %d",
        triggerZoneCount
    );

    ImGui::Text(
        "Monster Spawns: %d",
        monsterSpawnCount
    );

    ImGui::Text(
        "Music Gates: %d",
        musicGateCount
    );

    ImGui::Text(
        "Music NPCs: %d",
        musicNpcCount
    );

    ImGui::Separator();

    int errorCount =
        0;

    int warningCount =
        0;

    int infoCount =
        0;

    for (const SceneHealthEntry& entry : entries)
    {
        if (entry.level == SceneHealthLevel::Error)
        {
            errorCount++;
        }
        else if (entry.level == SceneHealthLevel::Warning)
        {
            warningCount++;
        }
        else
        {
            infoCount++;
        }
    }

    ImGui::Text(
        "Detailed Check: %d errors | %d warnings | %d OK",
        errorCount,
        warningCount,
        infoCount
    );

    if (ImGui::CollapsingHeader("Detailed Validation"))
    {
        for (const SceneHealthEntry& entry : entries)
        {
            ImGui::TextColored(
                GetSceneHealthColor(
                    entry.level
                ),
                "%s",
                GetSceneHealthPrefix(
                    entry.level
                )
            );

            ImGui::SameLine();

            ImGui::TextWrapped(
                "%s",
                entry.message.c_str()
            );
        }
    }

    ImGui::Separator();

    ImGui::TextDisabled(
        "This validator updates live while editing the scene."
    );

    ImGui::End();
}
bool ShouldSkipObjectByDistance(
    SceneObject* object,
    const glm::vec3& viewerPosition
)
{
    if (!useDistanceOptimization)
        return false;

    if (object == nullptr)
        return true;

    if (!object->visible)
        return true;

    if (IsImportantRenderObject(object))
        return false;

    glm::vec2 viewerXZ =
        glm::vec2(
            viewerPosition.x,
            viewerPosition.z
        );

    glm::vec2 objectXZ =
        glm::vec2(
            object->transform.position.x,
            object->transform.position.z
        );

    float distance =
        glm::length(
            viewerXZ -
            objectXZ
        );

    if (IsTinyPerformanceProp(object))
    {
        return distance > tinyPropRenderDistance;
    }

    if (IsSmallPerformanceProp(object))
    {
        return distance > smallPropRenderDistance;
    }

    if (IsRockPerformanceProp(object))
    {
        return distance > rockRenderDistance;
    }

    if (IsMediumPerformanceProp(object))
    {
        return distance > mediumPropRenderDistance;
    }

    return false;
}
bool IsTorchObject(
    SceneObject* object
);

bool IsTorchLightOn(
    SceneObject* object
);
glm::vec3 GetTorchLightOnColor()
{
    return glm::vec3(
        5.0f,
        2.6f,
        0.8f
    );
}
void ToggleTorchLight(
    SceneObject* object
);
bool IsGameplayInteractable(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (!object->visible)
        return false;

    if (object->name == "Player")
        return false;

    if (object->name == "Ground")
        return false;

    if (object->name == "Procedural Terrain")
        return false;

    if (object->name.find("Generated") != std::string::npos)
        return false;

    if (ObjectNameContains(object, "House"))
        return true;
    if (IsCampfireObject(object))
        return true;
    if (ObjectNameContains(object, "Camp"))
        return true;

    if (ObjectNameContains(object, "Rock"))
        return true;

    if (ObjectNameContains(object, "Bush"))
        return true;

    if (ObjectNameContains(object, "Wood Log"))
        return true;

    if (ObjectNameContains(object, "Tree Stump"))
        return true;

    if (ObjectNameContains(object, "Stone Wall"))
        return true;

    if (ObjectNameContains(object, "Fence"))
        return true;

    if (ObjectNameContains(object, "Path Tile"))
        return true;

    if (ObjectNameContains(object, "Wood Platform"))
        return true;

    if (IsTorchObject(object))
        return true;

    return false;
}

std::string GetInteractionActionText(
    SceneObject* object
)
{
    if (object == nullptr)
        return "interact";

    if (ObjectNameContains(object, "House"))
        return "inspect house";
    if (IsCampfireObject(object))
        return "rest at campfire";
    if (ObjectNameContains(object, "Camp"))
        return "inspect camp";

    if (ObjectNameContains(object, "Rock"))
        return "inspect rock";

    if (ObjectNameContains(object, "Bush"))
        return "inspect bush";

    if (ObjectNameContains(object, "Wood Log"))
        return "inspect log";

    if (ObjectNameContains(object, "Tree Stump"))
        return "inspect stump";

    if (ObjectNameContains(object, "Stone Wall"))
        return "inspect wall";

    if (ObjectNameContains(object, "Fence"))
        return "inspect fence";

    if (ObjectNameContains(object, "Path Tile"))
        return "inspect path";

    if (ObjectNameContains(object, "Wood Platform"))
        return "inspect platform";
    if (IsTorchObject(object))
    {
        if (IsTorchLightOn(object))
            return "turn off torch";

        return "turn on torch";
    }

    return "interact";
}
bool IsTorchObject(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (ObjectNameContains(object, "Torch"))
        return true;
    return false;
}

bool IsTorchLightOn(
    SceneObject* object
)
{
    if (object == nullptr)
        return false;

    if (object->attachedLight == nullptr)
        return false;

    return
        glm::length(
            object->attachedLight->color
        ) > 0.05f;
}

void ToggleTorchLight(
    SceneObject* object
)
{
    if (object == nullptr)
        return;

    if (object->attachedLight == nullptr)
        return;

    if (IsTorchLightOn(object))
    {
        object->attachedLight->color =
            glm::vec3(
                0.0f,
                0.0f,
                0.0f
            );
    }
    else
    {
        object->attachedLight->color =
            GetTorchLightOnColor();
    }
}
void UpdateTorchFireFlicker(
    Scene& scene,
    float currentTime
)
{
    if (!useTorchFireFlicker)
        return;

    for (SceneObject* object : scene.objects)
    {
        if (object == nullptr)
            continue;

        if (
            !IsTorchObject(object) &&
            !IsCampfireObject(object)
            )
        {
            continue;
        }
        if (object->attachedLight == nullptr)
            continue;

        if (!IsTorchLightOn(object))
            continue;

        float flickerA =
            std::sin(
                currentTime * 8.0f +
                object->transform.position.x * 0.37f
            );

        float flickerB =
            std::sin(
                currentTime * 15.0f +
                object->transform.position.z * 0.19f
            );

        float flicker =
            0.82f +
            flickerA * 0.12f +
            flickerB * 0.06f;

        flicker =
            glm::clamp(
                flicker,
                0.65f,
                1.15f
            );

        glm::vec3 baseColor =
            GetTorchLightOnColor();

        if (IsCampfireObject(object))
        {
            baseColor =
                glm::vec3(
                    7.0f,
                    3.5f,
                    1.0f
                );
        }

        object->attachedLight->color =
            glm::vec3(
                baseColor.r * flicker,
                baseColor.g * flicker,
                baseColor.b * (
                    0.85f +
                    flicker * 0.15f
                    )
            );
    }
}
std::string GetInteractionResultText(
    SceneObject* object
)
{
    if (object == nullptr)
        return "Nothing to interact with.";

    if (ObjectNameContains(object, "House"))
        return "House inspected. This object was placed from the editor.";
    if (IsCampfireObject(object))
        return "Campfire used. Dynamic fire light and interaction feedback are working.";
    if (ObjectNameContains(object, "Camp"))
        return "Camp inspected. This area was generated from a camp preset.";

    if (ObjectNameContains(object, "Rock"))
        return "Rock inspected. Collision and terrain placement are working.";

    if (ObjectNameContains(object, "Bush"))
        return "Bush inspected. Environment prop interaction works.";

    if (ObjectNameContains(object, "Wood Log"))
        return "Wood log inspected. Forest prop interaction works.";

    if (ObjectNameContains(object, "Tree Stump"))
        return "Tree stump inspected. Camp prop interaction works.";

    if (ObjectNameContains(object, "Stone Wall"))
        return "Stone wall inspected. Building piece interaction works.";

    if (ObjectNameContains(object, "Fence"))
        return "Fence inspected. Modular construction object detected.";

    if (ObjectNameContains(object, "Path Tile"))
        return "Path tile inspected. Layout piece detected.";

    if (ObjectNameContains(object, "Wood Platform"))
        return "Wood platform inspected. Buildable platform detected.";

    if (IsTorchObject(object))
    {
        if (IsTorchLightOn(object))
            return "Torch activated. Warm point light is now ON.";

        return "Torch disabled. Point light is now OFF.";
    }

    return "Object inspected.";
}

SceneObject* FindNearestInteractableObject(
    Scene& scene,
    const glm::vec3& playerPosition,
    float maxDistance
)
{
    SceneObject* nearestObject =
        nullptr;

    float nearestDistance =
        maxDistance;

    for (SceneObject* object : scene.objects)
    {
        if (!IsGameplayInteractable(object))
            continue;

        glm::vec3 objectPosition =
            object->transform.position;

        glm::vec2 playerXZ =
            glm::vec2(
                playerPosition.x,
                playerPosition.z
            );

        glm::vec2 objectXZ =
            glm::vec2(
                objectPosition.x,
                objectPosition.z
            );

        float distance =
            glm::length(
                playerXZ -
                objectXZ
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
// ================= MAIN =================
glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor =
        glfwGetPrimaryMonitor();

    int monitorX =
        0;

    int monitorY =
        0;

    int monitorWidth =
        1600;

    int monitorHeight =
        900;

    glfwGetMonitorWorkarea(
        primaryMonitor,
        &monitorX,
        &monitorY,
        &monitorWidth,
        &monitorHeight
    );

    glfwWindowHint(
        GLFW_DECORATED,
        GLFW_FALSE
    );

    glfwWindowHint(
        GLFW_RESIZABLE,
        GLFW_TRUE
    );

    GLFWwindow* window =
        glfwCreateWindow(
            monitorWidth,
            monitorHeight,
            "Orion",
            NULL,
            NULL
        );
       // full screen
 /*   GLFWmonitor* monitor =
        glfwGetPrimaryMonitor();

    const GLFWvidmode* videoMode =
        glfwGetVideoMode(
            monitor
        );

    glfwWindowHint(
        GLFW_RED_BITS,
        videoMode->redBits
    );

    glfwWindowHint(
        GLFW_GREEN_BITS,
        videoMode->greenBits
    );

    glfwWindowHint(
        GLFW_BLUE_BITS,
        videoMode->blueBits
    );

    glfwWindowHint(
        GLFW_REFRESH_RATE,
        videoMode->refreshRate
    );

    GLFWwindow* window =
        glfwCreateWindow(
            videoMode->width,
            videoMode->height,
            "Orion",
            monitor,
            NULL
        );*/
    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwSetWindowPos(
        window,
        monitorX,
        monitorY
    );
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }
    // ================= DEFAULT STARTUP ATMOSPHERE =================

    weatherSystem.enabled =
        true;

    // Foggy Forest
    weatherSystem.presetIndex =
        1;

    weatherSystem.showOverlay =
        true;

    dayNightSystem.timeOfDay =
        17.5f;

    dayNightSystem.enabled =
        false;
    Cubemap skybox(faces);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glEnable(GL_DEPTH_TEST);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);

    glfwSetCharCallback(
        window,
        ImGui_ImplGlfw_CharCallback
    );

    glfwSetKeyCallback(
        window,
        ImGui_ImplGlfw_KeyCallback
    );
    ImGui_ImplOpenGL3_Init("#version 330");
    glDisable(GL_CULL_FACE);
    TestAssimp();
    AnimationLibrary playerAnimations;
    InputManager::Init(window);
    playerAnimations.LoadAnimation(
        "Idle",
        "Assets/Models/Characters/Player/Idle.fbx"
    );

    playerAnimations.LoadAnimation(
        "Walk",
        "Assets/Models/Characters/Player/Walk.fbx"
    );

    playerAnimations.LoadAnimation(
        "Run",
        "Assets/Models/Characters/Player/Run.fbx"
    );
    mainMenuBackgroundTexture =
        LoadMenuTexture(
            "Assets/UI/orion_build_learn_create.png"
        );
    playerAnimations.LoadAnimation(
        "Jump",
        "Assets/Models/Characters/Player/Jump.fbx"
    );
 
    AssetDatabase::Initialize();
    // ================= SHADER CLASS =================
    unsigned int gizmoVAO, gizmoVBO;

    glGenVertexArrays(1, &gizmoVAO);
    // ================= COLLISION DEBUG CIRCLE BUFFER =================

    const int collisionCircleVertexCount =
        96;

    std::vector<float> collisionCircleVertices =
        CreateCollisionCircleVertices(
            collisionCircleVertexCount
        );

    unsigned int collisionCircleVAO =
        0;

    unsigned int collisionCircleVBO =
        0;

    glGenVertexArrays(
        1,
        &collisionCircleVAO
    );

    glGenBuffers(
        1,
        &collisionCircleVBO
    );

    glBindVertexArray(
        collisionCircleVAO
    );

    glBindBuffer(
        GL_ARRAY_BUFFER,
        collisionCircleVBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        collisionCircleVertices.size() * sizeof(float),
        collisionCircleVertices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(
        0
    );

    glBindVertexArray(
        0
    );
    glGenBuffers(1, &gizmoVBO);

    glBindVertexArray(gizmoVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoVertices), gizmoVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    Shader shader(vertexShaderSource, fragmentShaderSource);
    glm::vec3 lightPositions[] = {
    glm::vec3(2.0f, 2.0f, 2.0f),
    glm::vec3(-2.0f, 2.0f, 2.0f),
    glm::vec3(0.0f, 3.0f, -3.0f)
    };



    shader.setBool("isSelected", isSelected);
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f),
        glm::vec3(1.0f, 0.8f, 0.6f),
        glm::vec3(0.6f, 0.6f, 1.0f)
    };
    Renderer renderer;
    Scene scene;
    PlayerController playerController;
    ThirdPersonController thirdPersonController;
  

    AppMode appMode = AppMode::Editor;
    static int lightCounter = 1;
   /* Light* testLight = new Light();

  
    testLight->name =
        "Light_" +
        std::to_string(lightCounter++);

    testLight->position =
        glm::vec3(
            0.0f,
            3.0f,
            0.0f
        );

    scene.AddLight(testLight);
    testLight = new Light();

    testLight->name = "Light 1";

    testLight->position =
        glm::vec3(
            0.0f,
            3.0f,
            0.0f
        );


    scene.AddLight(testLight);*/
   
    Light* sun = new Light();

    sun->name = "Sun";

    sun->type = LightType::Directional;

    sun->direction =
        glm::normalize(
            glm::vec3(-0.2f, -1.0f, -0.3f));

    scene.AddLight(sun);
    Model myModel("character-a.obj");
    AnimatedModel testAnimatedPlayer(
        "Assets/Models/Characters/Player/Idle.fbx"
    );
    Model treeModel("character-human.obj");
   Model importedTree(
        "Assets/Models/Nature/Tree.obj"
    );
   Model pineTree4Model(
       "Assets/Models/Environment/NaturePack/PineTree_4.obj"
   );

   Model pineTree5Model(
       "Assets/Models/Environment/NaturePack/PineTree_5.obj"
   );

   Model commonTree3Model(
       "Assets/Models/Environment/NaturePack/CommonTree_3.obj"
   );

   Model commonTree4Model(
       "Assets/Models/Environment/NaturePack/CommonTree_4.obj"
   );

   Model birchTree3Model(
       "Assets/Models/Environment/NaturePack/BirchTree_3.obj"
   );

   Model willowTreeModel(
       "Assets/Models/Environment/NaturePack/Willow_1.obj"
   );

   Model willowTree2Model(
       "Assets/Models/Environment/NaturePack/Willow_2.obj"
   );

   Model rock4Model(
       "Assets/Models/Environment/NaturePack/Rock_4.obj"
   );

   Model rock5Model(
       "Assets/Models/Environment/NaturePack/Rock_5.obj"
   );

   Model rockMoss2Model(
       "Assets/Models/Environment/NaturePack/Rock_Moss_2.obj"
   );

   Model rockMoss3Model(
       "Assets/Models/Environment/NaturePack/Rock_Moss_3.obj"
   );

   Model bushBerriesModel(
       "Assets/Models/Environment/NaturePack/BushBerries_1.obj"
   );

   Model grass2Model(
       "Assets/Models/Environment/NaturePack/Grass_2.obj"
   );

   Model wheatModel(
       "Assets/Models/Environment/NaturePack/Wheat.obj"
   );
    Model forestEnvironment(
        "Assets/Models/Environment/terrain.obj"
    );
    Model grassTerrainModel(
        "Assets/Models/Environment/ForestDemo/Mineways2Skfb.obj",
        "Assets/Models/Environment/ForestDemo/"
    );
    Model pineTreeModel(
        "Assets/Models/Environment/NaturePack/PineTree_1.obj"
    );

    Model commonTreeModel(
        "Assets/Models/Environment/NaturePack/CommonTree_1.obj"
    );

    Model rockModel(
        "Assets/Models/Environment/NaturePack/Rock_1.obj"
    );

    Model bushModel(
        "Assets/Models/Environment/NaturePack/Bush_1.obj"
    );

    Model grassModel(
        "Assets/Models/Environment/NaturePack/Grass.obj"
    );
    Model grassPatchModel(
        "Assets/Models/Environment/NaturePack/Grass_Short.obj"
    );

    Model woodLogModel(
        "Assets/Models/Environment/NaturePack/WoodLog.obj"
    );

    Model pineTree3Model(
        "Assets/Models/Environment/NaturePack/PineTree_3.obj"
    );

    Model birchTreeModel(
        "Assets/Models/Environment/NaturePack/BirchTree_1.obj"
    );

    Model birchTree2Model(
        "Assets/Models/Environment/NaturePack/BirchTree_2.obj"
    );

    Model commonTree2Model(
        "Assets/Models/Environment/NaturePack/CommonTree_2.obj"
    );

    Model rock2Model(
        "Assets/Models/Environment/NaturePack/Rock_2.obj"
    );

    Model rock3Model(
        "Assets/Models/Environment/NaturePack/Rock_3.obj"
    );

    Model rockMossModel(
        "Assets/Models/Environment/NaturePack/Rock_Moss_1.obj"
    );

    Model plantModel(
        "Assets/Models/Environment/NaturePack/Plant_1.obj"
    );

    Model plant2Model(
        "Assets/Models/Environment/NaturePack/Plant_2.obj"
    );

    Model flowersModel(
        "Assets/Models/Environment/NaturePack/Flowers.obj"
    );

    Model treeStumpModel(
        "Assets/Models/Environment/NaturePack/TreeStump.obj"
    );
    Model pineTree2Model(
        "Assets/Models/Environment/NaturePack/PineTree_2.obj"
    );

    Model bush2Model(
        "Assets/Models/Environment/NaturePack/Bush_2.obj"
    );
    Model torchModel(
        "Assets/Models/Environment/Torch/Torch.obj",
        "Assets/Models/Environment/Torch/"
    );
    Model campfireModel(
        "Assets/Models/Environment/Campfire/campfire.obj",
        "Assets/Models/Environment/Campfire/"
    );
    Model woodenHouseModel(
        "Assets/Models/Environment/WoodenHouse/WoodenHouse.obj",
        "Assets/Models/Environment/WoodenHouse/"
    );
    Model newHouseModel(
        "Assets/Models/Environment/WoodenHouse/house2.obj",
        "Assets/Models/Environment/WoodenHouse/"
    );
    Model grassClumpModel(
        "Assets/Models/Environment/GrassClump/grass1.obj",
        "Assets/Models/Environment/GrassClump/"
    );

    Model flowerClumpModel(
        "Assets/Models/Environment/GrassClump/Flower.obj",
        "Assets/Models/Environment/GrassClump/"
    );
    //Model mountain1Model(
    //    "Assets/Models/Environment/Mountains/Mountain01.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    //Model mountain2Model(
    //    "Assets/Models/Environment/Mountains/Mountain02.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    //Model mountain3Model(
    //    "Assets/Models/Environment/Mountains/Mountain03.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    Shader skyboxShader(skyboxVertex, skyboxFragment);
    Shader gizmoShader(gizmoVertexShader, gizmoFragmentShader);
    Shader gridShader(gridVertexShader, gridFragmentShader);

    GridRenderer grid;
    // ================= CUBE DATA =================
    float vertices[] = {
        // positions          // normals           // texcoords

        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
         0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
        -0.5f, 0.5f,-0.5f, 0,0,-1, 0,1,
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,


        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,
         0.5f,-0.5f, 0.5f, 0,0,1, 1,0,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
        -0.5f, 0.5f, 0.5f, 0,0,1, 0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,

        -0.5f, 0.5f, 0.5f, -1,0,0, 1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0, 1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,1,
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0, 0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0, 1,0,

         0.5f, 0.5f, 0.5f, 1,0,0, 1,0,
         0.5f, 0.5f,-0.5f, 1,0,0, 1,1,
         0.5f,-0.5f,-0.5f, 1,0,0, 0,1,
         0.5f,-0.5f,-0.5f, 1,0,0, 0,1,
         0.5f,-0.5f, 0.5f, 1,0,0, 0,0,
         0.5f, 0.5f, 0.5f, 1,0,0, 1,0,

        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,
         0.5f,-0.5f,-0.5f, 0,-1,0, 1,1,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,0,
        -0.5f,-0.5f, 0.5f, 0,-1,0, 0,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,

        -0.5f, 0.5f,-0.5f, 0,1,0, 0,1,
         0.5f, 0.5f,-0.5f, 0,1,0, 1,1,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,0,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,0,
        -0.5f, 0.5f, 0.5f, 0,1,0, 0,0,
        -0.5f, 0.5f,-0.5f, 0,1,0, 0,1
    };

    float skyboxVertices[] = {

-1.0f,  1.0f, -1.0f,
-1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f, -1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f, -1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

-1.0f,  1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f,  1.0f
    };
    unsigned int skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    Texture containerTexture("container.jpg");
    Texture terrainGrassTexture(
        "textures/terrain/grass1.jpg"
    );

    Texture terrainDirtTexture(
        "textures/terrain/dirt.jpg"
    );

    Texture terrainCliffTexture(
        "textures/terrain/cliff.jpg"
    );

    Material cubeMaterial(&containerTexture);
    Material groundMaterial(
        &containerTexture
    );
    Mesh cube(vertices, sizeof(vertices));
    std::vector<float> proceduralTerrainVertices;

    BuildProceduralTerrain(
        proceduralTerrainVertices,
        520.0f,
        170
    );

    Mesh proceduralTerrainMesh(
        proceduralTerrainVertices.data(),
        proceduralTerrainVertices.size() * sizeof(float)
    );
    Material proceduralTerrainMaterial(
        nullptr
    );
    proceduralTerrainMaterial.tint =
        glm::vec3(
            1.0f,
            1.0f,
            1.0f
        );

    proceduralTerrainMaterial.ambient =
        glm::vec3(
            0.45f,
            0.45f,
            0.45f
        );

    proceduralTerrainMaterial.diffuse =
        glm::vec3(
            1.0f,
            1.0f,
            1.0f
        );

    proceduralTerrainMaterial.specular =
        glm::vec3(
            0.05f,
            0.05f,
            0.05f
        );

    proceduralTerrainMaterial.shininess =
        4.0f;
    SceneObject proceduralTerrainObject(
        &proceduralTerrainMesh,
        &shader,
        &proceduralTerrainMaterial
    );

    proceduralTerrainObject.name =
        "Procedural Terrain";
    SetObjectMetadata(
        &proceduralTerrainObject,
        "procedural_terrain",
        AssetType::Terrain,
        SpawnSource::Procedural,
        false,
        true
    );
    proceduralTerrainObject.transform.position =
        glm::vec3(
            0.0f
        );

    proceduralTerrainObject.transform.scale =
        glm::vec3(
            1.0f
        );

    proceduralTerrainObject.boundingRadius =
        800.0f;

    proceduralTerrainObject.isCollider =
        false;

    scene.AddObject(
        &proceduralTerrainObject
    );
    SceneObject cube1(&cube, &shader, &cubeMaterial);
    SceneObject cube2(&cube, &shader, &cubeMaterial);
    SceneObject cube3(&cube, &shader, &cubeMaterial);
    SceneObject ground(
        &cube,
        &shader,
        &groundMaterial
    );
    cube1.name = "Cube 1";
    cube2.name = "Cube 2";
    cube3.name = "Cube 3";
    ground.name = "Ground";
    cube1.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube2.transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
    cube3.transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    ground.transform.position =
        glm::vec3(
            0.0f,
            -0.55f,
            -10.0f
        );

    ground.transform.scale =
        glm::vec3(
            120.0f,
            0.1f,
            120.0f
        );
    ground.material->tint =
        glm::vec3(
            0.25f,
            0.45f,
            0.20f
        );
    cube2.transform.scale = glm::vec3(1.5f);
    cube3.transform.scale = glm::vec3(0.5f);
    ground.visible = false;
    ground.isCollider = false;
    scene.AddObject( &ground  );
   
    auto AddEnvironmentModel =
        [&](Model* model,
            const std::string& name,
            glm::vec3 position,
            glm::vec3 scale,
            bool collider)
        {
            SceneObject* obj =
                new SceneObject(
                    model,
                    &shader
                );

            obj->name = name;
            obj->assetId =
                name;

            obj->assetType =
                AssetType::Prop;

            obj->spawnSource =
                SpawnSource::Procedural;

            obj->persistent =
                false;

            obj->showInHierarchy =
                true;
            obj->transform.position =
                position;

            obj->transform.scale =
                scale;

            obj->boundingRadius =
                50.0f;

            obj->isCollider =
                collider;
            obj->colliderRadius =
                glm::max(
                    scale.x,
                    scale.z
                ) * 0.4f;
            scene.AddObject(obj);

            return obj;
        };

    float grassTileSpacing =
        8.0f;
    bool useOldGrassTiles =
        false;

    if (useOldGrassTiles)
    {
        for (int x = -10; x <= 10; x++)
        {
            for (int z = -10; z <= 10; z++)
            {
                AddEnvironmentModel(
                    &grassTerrainModel,
                    "Grass Terrain Tile",
                    glm::vec3(
                        x * grassTileSpacing,
                        -0.35f,
                        z * grassTileSpacing
                    ),
                    glm::vec3(
                        0.35f
                    ),
                    false
                );
            }
        }
    }
//// Trees
//    AddEnvironmentModel(
//        &pineTreeModel,
//        "Pine Tree 1",
//        glm::vec3(-8.0f, 0.05f, -12.0f),
//        glm::vec3(0.35f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &pineTreeModel,
//        "Pine Tree 2",
//        glm::vec3(7.0f, 0.0f, -14.0f),
//        glm::vec3(1.0f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &commonTreeModel,
//        "Common Tree 1",
//        glm::vec3(-12.0f, 0.0f, -4.0f),
//        glm::vec3(0.9f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &commonTreeModel,
//        "Common Tree 2",
//        glm::vec3(10.0f, 0.0f, -6.0f),
//        glm::vec3(1.1f),
//        false
//    );
//
//    // Rocks
//    AddEnvironmentModel(
//        &rockModel,
//        "Rock 1",
//        glm::vec3(-4.0f, 0.0f, -8.0f),
//        glm::vec3(1.0f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &rockModel,
//        "Rock 2",
//        glm::vec3(5.0f, 0.0f, -10.0f),
//        glm::vec3(0.7f),
//        true
//    );
//
//    // Bushes
//    AddEnvironmentModel(
//        &bushModel,
//        "Bush 1",
//        glm::vec3(-2.0f, 0.0f, -5.0f),
//        glm::vec3(0.8f),
//        false
//    );
//
//    AddEnvironmentModel(
//        &bushModel,
//        "Bush 2",
//        glm::vec3(3.0f, 0.0f, -7.0f),
//        glm::vec3(0.9f),
//        true
//    );
//
//    // Grass patches
//    AddEnvironmentModel(
//        &grassModel,
//        "Grass Patch 1",
//        glm::vec3(0.0f, 0.0f, -6.0f),
//        glm::vec3(1.0f),
//        false
//    );
//
//    AddEnvironmentModel(
//        &grassModel,
//        "Grass Patch 2",
//        glm::vec3(6.0f, 0.0f, -3.0f),
//        glm::vec3(1.2f),
//        true
//    );
//
//    // Wood log
//    AddEnvironmentModel(
//        &woodLogModel,
//        "Wood Log",
//        glm::vec3(-6.0f, 0.0f, -6.0f),
//        glm::vec3(1.0f),
//        true
//    );
    // ================= EXTRA FOREST DECORATION =================

// Trees
    //AddEnvironmentModel(
    //    &pineTree2Model,
    //    "Pine Tree 3",
    //    glm::vec3(-18.0f, 0.05f, -18.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &pineTree3Model,
    //    "Pine Tree 4",
    //    glm::vec3(18.0f, 0.05f, -22.0f),
    //    glm::vec3(0.38f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &birchTreeModel,
    //    "Birch Tree 1",
    //    glm::vec3(-22.0f, 0.05f, 8.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &birchTree2Model,
    //    "Birch Tree 2",
    //    glm::vec3(22.0f, 0.05f, 10.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &commonTree2Model,
    //    "Common Tree 3",
    //    glm::vec3(0.0f, 0.05f, -25.0f),
    //    glm::vec3(0.40f),
    //    false
    //);

    //// Rocks
    //AddEnvironmentModel(
    //    &rock2Model,
    //    "Rock 3",
    //    glm::vec3(-10.0f, 0.05f, 6.0f),
    //    glm::vec3(0.35f),
    //    true
    //);

    //AddEnvironmentModel(
    //    &rock3Model,
    //    "Rock 4",
    //    glm::vec3(12.0f, 0.05f, -4.0f),
    //    glm::vec3(0.35f),
    //    true
    //);

    //AddEnvironmentModel(
    //    &rockMossModel,
    //    "Moss Rock",
    //    glm::vec3(4.0f, 0.05f, 14.0f),
    //    glm::vec3(0.40f),
    //    true
    //);

    //// Plants / flowers
    //AddEnvironmentModel(
    //    &plantModel,
    //    "Plant 1",
    //    glm::vec3(-5.0f, 0.05f, 10.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &plant2Model,
    //    "Plant 2",
    //    glm::vec3(8.0f, 0.05f, 12.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &flowersModel,
    //    "Flowers",
    //    glm::vec3(2.0f, 0.05f, 8.0f),
    //    glm::vec3(0.45f),
    //    false
    //);

    //// Stump
    //AddEnvironmentModel(
    //    &treeStumpModel,
    //    "Tree Stump",
    //    glm::vec3(-14.0f, 0.05f, -2.0f),
    //    glm::vec3(0.45f),
    //    true
    //);
    // ================= GRASS FIELD =================

   /* for (int x = -8; x <= 8; x++)
    {
        for (int z = -8; z <= 8; z++)
        {
            float worldX =
                x * 4.0f;

            float worldZ =
                z * 4.0f;

            float worldY =
                TerrainGenerator::GetHeight(
                    worldX,
                    worldZ
                );

            AddEnvironmentModel(
                &grassPatchModel,
                "Grass Patch",
                glm::vec3(
                    worldX,
                    worldY + 0.05f,
                    worldZ
                ),
                glm::vec3(
                    0.7f
                ),
                false
            );
        }
    }*/
    // ================= REAL MOUNTAIN BORDER =================

    //std::vector<Model*> mountainModels =
    //{
    //    &mountain1Model,
    //    &mountain2Model,
    //    &mountain3Model
    //};

    //auto AddBorderMountain =
    //    [&](float x, float z, float scale)
    //    {
    //        Model* selectedMountain =
    //            mountainModels[
    //                rand() % mountainModels.size()
    //            ];

    //        SceneObject* mountain =
    //            AddEnvironmentModel(
    //                selectedMountain,
    //                "Border Mountain",
    //                glm::vec3(
    //                    x,
    //                    -2.0f,
    //                    z
    //                ),
    //                glm::vec3(
    //                    scale
    //                ),
    //                false
    //            );

    //        mountain->boundingRadius =
    //            500.0f;

    //        return mountain;
    //    };
    //// Back border mountains
    //for (int i = -4; i <= 4; i++)
    //{
    //    AddBorderMountain(
    //        i * 35.0f,
    //        -120.0f,
    //        8.0f
    //    );
    //}

    //// Left border mountains
    //for (int i = -3; i <= 3; i++)
    //{
    //    AddBorderMountain(
    //        -120.0f,
    //        i * 35.0f,
    //        8.0f
    //    );
    //}

    //// Right border mountains
    //for (int i = -3; i <= 3; i++)
    //{
    //    AddBorderMountain(
    //        120.0f,
    //        i * 35.0f,
    //        8.0f
    //    );
    //}
    // ================= TEST ONE MOUNTAIN =================

   /* SceneObject* testMountain =
        new SceneObject(
            &mountain2Model,
            &shader
        );

    testMountain->name =
        "Test Mountain";

    testMountain->transform.position =
        glm::vec3(
            0.0f,
            -2.0f,
            -80.0f
        );

    testMountain->transform.scale =
        glm::vec3(
            0.02f
        );

    testMountain->boundingRadius =
        500.0f;

    testMountain->isCollider =
        false;*/

  /*  scene.AddObject(
        testMountain
    );*/

    // ================= FOREST GENERATOR V1 =================
    // ================= WORLD BORDER MOUNTAINS =================

    srand(7);

    auto RandomRange =
        [](float minValue, float maxValue)
        {
            float t =
                (float)(rand() % 1000) / 1000.0f;

            return minValue +
                t * (maxValue - minValue);
        };

    std::vector<Model*> treeModels =
    {
        &pineTreeModel,
        &pineTree2Model,
        &pineTree3Model,
        &pineTree4Model,
        &pineTree5Model,

        &commonTreeModel,
        &commonTree2Model,
        &commonTree3Model,
        &commonTree4Model,

        &birchTreeModel,
        &birchTree2Model,
        &birchTree3Model,

        &willowTreeModel,
        &willowTree2Model
    };

    std::vector<Model*> rockModels =
    {
        &rockModel,
        &rock2Model,
        &rock3Model,
        &rock4Model,
        &rock5Model,

        &rockMossModel,
        &rockMoss2Model,
        &rockMoss3Model
    };

    std::vector<Model*> plantModels =
    {
        &bushModel,
        &bush2Model,
        &bushBerriesModel,

        &plantModel,
        &plant2Model,

        &flowersModel,
        &grassModel,
        &grass2Model,
        &wheatModel
    };
    auto IsFlatEnoughForCamp =
        [&](float x, float z)
        {
            float center =
                GetTerrainHeight(
                    x,
                    z
                );

            float right =
                GetTerrainHeight(
                    x + 4.0f,
                    z
                );

            float left =
                GetTerrainHeight(
                    x - 4.0f,
                    z
                );

            float front =
                GetTerrainHeight(
                    x,
                    z + 4.0f
                );

            float back =
                GetTerrainHeight(
                    x,
                    z - 4.0f
                );

            float maxDifference =
                std::max(
                    std::max(
                        std::abs(center - right),
                        std::abs(center - left)
                    ),
                    std::max(
                        std::abs(center - front),
                        std::abs(center - back)
                    )
                );

            return
                maxDifference < 2.8f;
        };
    auto AddObjectOnTerrain =
        [&](Model* model,
            const std::string& name,
            float x,
            float z,
            glm::vec3 scale,
            bool collider,
            AssetType assetType,
            const std::string& assetId,
            bool showInHierarchy,
            float terrainOffset = 0.05f)
        {
            float y =
                GetObjectTerrainY(
                    x,
                    z,
                    terrainOffset
                );

            SceneObject* object =
                AddEnvironmentModel(
                    model,
                    name,
                    glm::vec3(
                        x,
                        y,
                        z
                    ),
                    scale,
                    collider
                );

            object->transform.rotation.y =
                RandomRange(
                    0.0f,
                    360.0f
                );

            SetObjectMetadata(
                object,
                assetId,
                assetType,
                SpawnSource::Procedural,
                false,
                showInHierarchy
            );

            return object;
        };
   

    int grassClumpCounter =
        1;

    int flowerClumpCounter =
        1;

    int generatedTreeCounter =
        1;

    int generatedRockCounter =
        1;

    int generatedPlantCounter =
        1;

    int generatedLogCounter =
        1;

    int borderTreeCounter =
        1;

    int rockClusterCounter =
        1;
    int campCounter =
        1;

    int campHouseCounter =
        1;

    int campTreeCounter =
        1;

    int campRockCounter =
        1;
    int campfireCounter =
        1;
    int forestZoneCounter =
        1;

    int forestZoneTreeCounter =
        1;
    int manualHouseCounter =
        1;
    auto BuildCamp =
        [&](float centerX,
            float centerZ,
            bool useNewHouse)
        {
            if (!IsFlatEnoughForCamp(centerX, centerZ))
            {
                bool foundFlatPlace =
                    false;

                for (int attempt = 0; attempt < 25; attempt++)
                {
                    float angle =
                        RandomRange(
                            0.0f,
                            6.28f
                        );

                    float distance =
                        RandomRange(
                            6.0f,
                            28.0f
                        );

                    float testX =
                        centerX +
                        std::cos(angle) * distance;

                    float testZ =
                        centerZ +
                        std::sin(angle) * distance;

                    if (IsFlatEnoughForCamp(testX, testZ))
                    {
                        centerX =
                            testX;

                        centerZ =
                            testZ;

                        foundFlatPlace =
                            true;

                        break;
                    }
                }

                if (!foundFlatPlace)
                {
                    std::cout
                        << "Camp placement failed: no flat terrain nearby."
                        << std::endl;

                    return;
                }
            }
            Model* selectedHouse =
                useNewHouse
                ? &newHouseModel
                : &woodenHouseModel;

            glm::vec3 selectedHouseScale =
                useNewHouse
                ? glm::vec3(0.01f)
                : glm::vec3(0.8f);

            float selectedHouseTerrainOffset =
                useNewHouse
                ? -0.55f
                : 0.05f;

            std::string selectedHouseAssetId =
                useNewHouse
                ? "house_2"
                : "wooden_house";

            std::string selectedHouseName =
                useNewHouse
                ? "Camp House 2 "
                : "Camp House 1 ";

            SceneObject* campHouse =
                AddObjectOnTerrain(
                    selectedHouse,
                    selectedHouseName + std::to_string(campHouseCounter++),
                    centerX,
                    centerZ,
                    selectedHouseScale,
                    true,
                    AssetType::House,
                    selectedHouseAssetId,
                    true,
                    selectedHouseTerrainOffset
                );

            campHouse->boundingRadius =
                120.0f;

            campHouse->colliderRadius =
                7.0f;
     

            float campfireX =
                centerX;

            float campfireZ =
                centerZ + 12.0f;

            if (!IsGoodTerrainSpawnPoint(campfireX, campfireZ))
            {
                campfireZ =
                    centerZ - 12.0f;
            }

            if (!IsGoodTerrainSpawnPoint(campfireX, campfireZ))
            {
                campfireX =
                    centerX + 12.0f;

                campfireZ =
                    centerZ;
            }

            if (!IsGoodTerrainSpawnPoint(campfireX, campfireZ))
            {
                campfireX =
                    centerX - 12.0f;

                campfireZ =
                    centerZ;
            }
            SceneObject* campfireObject =
                AddObjectOnTerrain(
                    &campfireModel,
                    "Campfire " + std::to_string(campfireCounter++),
                    campfireX,
                    campfireZ,
                    glm::vec3(
                        0.8f
                    ),
                    true,
                    AssetType::Prop,
                    "campfire",
                    true,
                    0.05f
                );

            campfireObject->boundingRadius =
                30.0f;

            campfireObject->colliderRadius =
                1.8f;

            Light* campfireLight =
                new Light();

            campfireLight->name =
                "Campfire Light " +
                std::to_string(campfireCounter);

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
                    7.0f,
                    3.5f,
                    1.0f
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
            // Trees around the camp
            for (int i = 0; i < 10; i++)
            {
                float angle =
                    RandomRange(
                        0.0f,
                        6.28f
                    );

                float distance =
                    RandomRange(
                        12.0f,
                        28.0f
                    );

                float x =
                    centerX +
                    std::cos(angle) * distance;

                float z =
                    centerZ +
                    std::sin(angle) * distance;

                Model* chosenTree =
                    treeModels[
                        rand() % treeModels.size()
                    ];

                SceneObject* campTree =
                    AddObjectOnTerrain(
                        chosenTree,
                        "Camp Tree " + std::to_string(campTreeCounter++),
                        x,
                        z,
                        glm::vec3(
                            RandomRange(
                                1.2f,
                                2.0f
                            )
                        ),
                        false,
                        AssetType::Tree,
                        "camp_tree",
                        false
                    );

                campTree->boundingRadius =
                    80.0f;
            }

            // Rocks around the camp
            for (int i = 0; i < 7; i++)
            {
                float angle =
                    RandomRange(
                        0.0f,
                        6.28f
                    );

                float distance =
                    RandomRange(
                        8.0f,
                        22.0f
                    );

                float x =
                    centerX +
                    std::cos(angle) * distance;

                float z =
                    centerZ +
                    std::sin(angle) * distance;

                Model* chosenRock =
                    rockModels[
                        rand() % rockModels.size()
                    ];

                SceneObject* campRock =
                    AddObjectOnTerrain(
                        chosenRock,
                        "Camp Rock " + std::to_string(campRockCounter++),
                        x,
                        z,
                        glm::vec3(
                            RandomRange(
                                1.0f,
                                1.8f
                            )
                        ),
                        true,
                        AssetType::Rock,
                        "camp_rock",
                        true
                    );

                campRock->colliderRadius =
                    1.8f;
            }

            // Logs and stump decorations
            for (int i = 0; i < 4; i++)
            {
                float angle =
                    RandomRange(
                        0.0f,
                        6.28f
                    );

                float distance =
                    RandomRange(
                        5.0f,
                        14.0f
                    );

                float x =
                    centerX +
                    std::cos(angle) * distance;

                float z =
                    centerZ +
                    std::sin(angle) * distance;

                Model* chosenProp =
                    rand() % 2 == 0
                    ? &woodLogModel
                    : &treeStumpModel;

                AddObjectOnTerrain(
                    chosenProp,
                    "Camp Prop " + std::to_string(campCounter++),
                    x,
                    z,
                    glm::vec3(
                        RandomRange(
                            0.9f,
                            1.4f
                        )
                    ),
                    true,
                    AssetType::Prop,
                    "camp_prop",
                    true
                );
            }
        };
        auto BuildForestZone =
            [&](float centerX,
                float centerZ,
                float radius,
                int treeCount)
            {
                for (int i = 0; i < treeCount; i++)
                {
                    float angle =
                        RandomRange(
                            0.0f,
                            6.28f
                        );

                    float distance =
                        RandomRange(
                            0.0f,
                            radius
                        );

                    float x =
                        centerX +
                        std::cos(angle) * distance;

                    float z =
                        centerZ +
                        std::sin(angle) * distance;

                    if (!IsGoodTerrainSpawnPoint(x, z))
                        continue;

                    Model* chosenTree =
                        treeModels[
                            rand() % treeModels.size()
                        ];

                    SceneObject* tree =
                        AddObjectOnTerrain(
                            chosenTree,
                            "Forest Zone Tree " + std::to_string(forestZoneTreeCounter++),
                            x,
                            z,
                            glm::vec3(
                                RandomRange(
                                    1.4f,
                                    2.3f
                                )
                            ),
                            false,
                            AssetType::Tree,
                            "forest_zone_tree",
                            false
                        );

                    tree->boundingRadius =
                        90.0f;
                }
            };
    // ================= SIMPLE LOW-POLY GRASS FIELD =================

    for (int i = 0; i < 160; i++)
    {
        float x =
            RandomRange(
                -240.0f,
                240.0f
            );

        float z =
            RandomRange(
                -240.0f,
                240.0f
            );

        if (!IsGoodTerrainSpawnPoint(x, z))
            continue;

        float y =
            GetObjectTerrainY(
                x,
                z,
                0.02f
            );

        SceneObject* grassObject =
            AddEnvironmentModel(
                &grassClumpModel,
                "Grass Clump " + std::to_string(grassClumpCounter++),
                glm::vec3(
                    x,
                    y,
                    z
                ),
                glm::vec3(
                    RandomRange(
                        0.8f,
                        1.3f
                    )
                ),
                false
            );

        grassObject->transform.rotation =
            glm::vec3(
                0.0f,
                RandomRange(
                    0.0f,
                    360.0f
                ),
                0.0f
            );

        grassObject->boundingRadius =
            8.0f;
        SetObjectMetadata(
            grassObject,
            "grass_clump",
            AssetType::Grass,
            SpawnSource::Procedural,
            false,
            false
        );
    }

    // ================= SIMPLE FLOWER PATCHES =================

    for (int i = 0; i < 70; i++)
    {
        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        if (!IsGoodTerrainSpawnPoint(x, z))
            continue;

        float y =
            GetObjectTerrainY(
                x,
                z,
                0.03f
            );

        SceneObject* flowerObject =
            AddEnvironmentModel(
                &flowerClumpModel,
                "Flower Clump " + std::to_string(flowerClumpCounter++),
                glm::vec3(
                    x,
                    y,
                    z
                ),
                glm::vec3(
                    RandomRange(
                        1.0f,
                        1.6f
                    )
                ),
                false
            );

        flowerObject->transform.rotation =
            glm::vec3(
                0.0f,
                RandomRange(
                    0.0f,
                    360.0f
                ),
                0.0f
            );

        flowerObject->boundingRadius =
            8.0f;
        SetObjectMetadata(
            flowerObject,
            "flower_clump",
            AssetType::Flower,
            SpawnSource::Procedural,
            false,
            false
        );
    }
    // ================= PROCEDURAL GRASS AND FLOWERS =================

    // Trees
    for (int i = 0; i < 30; i++)
    {
        Model* chosenTree =
            treeModels[
                rand() % treeModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        float scale =
            RandomRange(
                1.35f,
                2.10f
            );
        SceneObject* treeObject =
            AddEnvironmentModel(
                chosenTree,
                "Generated Tree " + std::to_string(generatedTreeCounter++),
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );

        SetObjectMetadata(
            treeObject,
            "generated_tree",
            AssetType::Tree,
            SpawnSource::Procedural,
            false,
            false
        );
    }

    // Rocks
    for (int i = 0; i < 20; i++)
    {
        Model* chosenRock =
            rockModels[
                rand() % rockModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        float scale =
            RandomRange(
                1.20f,
                2.00f
            );

        SceneObject* rockObject =
            AddEnvironmentModel(
                chosenRock,
                "Generated Rock " + std::to_string(generatedRockCounter++),
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                true
            );

        SetObjectMetadata(
            rockObject,
            "generated_rock",
            AssetType::Rock,
            SpawnSource::Procedural,
            false,
            true
        );
    }

    // Bushes / plants / flowers
    for (int i = 0; i < 100; i++)
    {
        Model* chosenPlant =
            plantModels[
                rand() % plantModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
          RandomRange(
    -220.0f,
    220.0f
);

        float scale =
            RandomRange(
                0.50f,
                0.85f
            );

        SceneObject* plantObject =
            AddEnvironmentModel(
                chosenPlant,
                "Generated Plant " + std::to_string(generatedPlantCounter++),
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );

        SetObjectMetadata(
            plantObject,
            "generated_plant",
            AssetType::Bush,
            SpawnSource::Procedural,
            false,
            false
        );
    }

    // Logs / stumps
    for (int i = 0; i < 20; i++)
    {
        Model* chosenObject =
            (rand() % 2 == 0)
            ? &woodLogModel
            : &treeStumpModel;

        float x =
            RandomRange(
                -200.0f,
                200.0f
            );

        float z =
            RandomRange(
                -200.0f,
                200.0f
            );

        float scale =
            RandomRange(
                0.80f,
                1.25f
            );

        SceneObject* logObject =
            AddEnvironmentModel(
                chosenObject,
                "Generated Log/Stump " + std::to_string(generatedLogCounter++),
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                true
            );

        SetObjectMetadata(
            logObject,
            "generated_log_stump",
            AssetType::Prop,
            SpawnSource::Procedural,
            false,
            true
        );
    }
    // ================= GENERATED CAMPS =================

    BuildCamp(
        -45.0f,
        -20.0f,
        false
    );

    BuildCamp(
        55.0f,
        35.0f,
        true
    );

    BuildCamp(
        10.0f,
        85.0f,
        false
    );

    // ================= FOREST ZONES =================

    BuildForestZone(
        -75.0f,
        45.0f,
        35.0f,
        28
    );

    BuildForestZone(
        80.0f,
        -55.0f,
        40.0f,
        32
    );

    BuildForestZone(
        5.0f,
        -95.0f,
        32.0f,
        24
    );
    // ================= FOREST BORDER WALL =================

    auto AddBorderTree =
        [&](float x, float z)
        {
            Model* chosenTree =
                treeModels[
                    rand() % treeModels.size()
                ];

            float scale =
                RandomRange(
                    1.1f,
                    1.7f
                );
            SceneObject* borderTreeObject =
                AddEnvironmentModel(
                    chosenTree,
                    "Border Tree " + std::to_string(borderTreeCounter++),
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );
            SetObjectMetadata(
                borderTreeObject,
                "border_tree",
                AssetType::Tree,
                SpawnSource::Procedural,
                false,
                false
            );
        };

    // Back forest border
    for (int i = -14; i <= 14; i++)
    {
        AddBorderTree(
            i * 7.0f,
            -85.0f + RandomRange(-3.0f, 3.0f)
        );
    }

    // Left forest border
    for (int i = -12; i <= 12; i++)
    {
        AddBorderTree(
            -85.0f + RandomRange(-3.0f, 3.0f),
            i * 7.0f
        );
    }

    // Right forest border
    for (int i = -12; i <= 12; i++)
    {
        AddBorderTree(
            85.0f + RandomRange(-3.0f, 3.0f),
            i * 7.0f
        );
    }
    // ================= EXTRA ROCK CLUSTERS =================

    for (int cluster = 0; cluster < 12; cluster++)
    {
        float centerX =
            RandomRange(
                -70.0f,
                70.0f
            );

        float centerZ =
            RandomRange(
                -70.0f,
                70.0f
            );

        int rocksInCluster =
            3 + rand() % 4;

        for (int i = 0; i < rocksInCluster; i++)
        {
            Model* chosenRock =
                rockModels[
                    rand() % rockModels.size()
                ];

            float rockX =
                centerX + RandomRange(-3.0f, 3.0f);

            float rockZ =
                centerZ + RandomRange(-3.0f, 3.0f);

            SceneObject* clusterRockObject =
                AddEnvironmentModel(
                    chosenRock,
                    "Rock Cluster " + std::to_string(rockClusterCounter++),
                    glm::vec3(
                        rockX,
                        GetTerrainHeight(rockX, rockZ) + 0.05f,
                        rockZ
                    ),
                    glm::vec3(
                        RandomRange(0.6f, 1.2f)
                    ),
                    true
                );

            SetObjectMetadata(
                clusterRockObject,
                "rock_cluster",
                AssetType::Rock,
                SpawnSource::Procedural,
                false,
                true
            );
        }
    }
    playerObject =
        new SceneObject(
            &myModel,
            &shader
        );
    playerObject->name = "Player";
    SetObjectMetadata(
        playerObject,
        "player",
        AssetType::Player,
        SpawnSource::Manual,
        true,
        true
    );
    playerObject->transform.position =
        playerSpawnPosition;
    playerObject->transform.position.y =
        GetPlayerTerrainY(
            playerObject->transform.position.x,
            playerObject->transform.position.z
        );
    playerSpawnPosition =
        playerObject->transform.position;
    playerObject->transform.scale =
        glm::vec3(
            0.6f
        );

    playerObject->boundingRadius = 3.0f;

    scene.AddObject(playerObject);
    /*SceneObject* environmentObject =
        new SceneObject(
            &forestEnvironment,
            &shader
        );

    environmentObject->name =
        "Forest Cabin Environment";

    environmentObject->transform.position =
        glm::vec3(
            0.0f,
            0.0f,
            -10.0f
        );

    environmentObject->transform.scale =
        glm::vec3(
            1.0f
        );

    environmentObject->boundingRadius =
        300.0f;
    environmentObject->isCollider = false;
    scene.AddObject(
        environmentObject
    );*/
    
    /*ground.boundingRadius = 100.0f;
    SceneObject* treeObject =
        new SceneObject(
            &importedTree,
            &shader
        );

    treeObject->name = "Imported Tree";
    treeObject->boundingRadius = 20.0f;

    treeObject->transform.position =
        glm::vec3(
            0.0f,
            0.0f,
            -6.0f
        );

    treeObject->transform.scale =
        glm::vec3(
            0.1f
        );

    scene.AddObject(treeObject);*/
   /* std::vector<SceneObject*> manyCubes;

    for (int i = 0; i < 100; i++)
    {
        SceneObject* obj = new SceneObject(&cube, &shader, &cubeMaterial);
        obj->name = "Cube_" + std::to_string(i);
        float x = (rand() % 50 - 25);
        float y = (rand() % 10 - 5);
        float z = -(rand() % 50);

        obj->transform.position = glm::vec3(x, y, z);

        scene.AddObject(obj);
        manyCubes.push_back(obj);
    }*/

    /*glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);*/

    // ================= TEXTURE =================

    unsigned int texture;
    //glGenTextures(1, &texture);
   //glBindTexture(GL_TEXTURE_2D, texture);
   //Texture containerTexture("D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\container.jpg");
      /*  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

    int width =
        0;

    int height =
        0;

    int nrChannels =
        0;
    std::cout << "==== CONTROLS ====\n";
    std::cout << "WASD - Move camera\n";
    std::cout << "Mouse - Look around\n";
    std::cout << "Left Click - Select object\n";
    std::cout << "N - Add cube\n";
    std::cout << "M - Delete object\n";
    std::cout << "Arrows/PageUp/PageDown - Move object\n";
    std::cout << "IJKLUO - Rotate object\n";
    std::cout << "+/- - Scale object\n";
    std::cout << "C - Toggle frustum culling\n";
    std::cout << "===================\n";
    // unsigned char* data = stbi_load("D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\container.jpg", &width, &height, &nrChannels, 0);
    Shader lightShader(lightVertexSource, lightFragmentSource);
    shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setBool("useTexture", true);
    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };


    // ================= RENDER LOOP =================

    cube1.AddChild(&cube2);
    cube2.AddChild(&cube3);
    scene.objects[0]->AddChild(scene.objects[1]);
    // ===== ATTACH COMPONENTS =====
    cube1.AddComponent(new RotatorComponent(glm::vec3(0.0f, 1.0f, 0.0f), 50.0f));
    cube2.AddComponent(new RotatorComponent(glm::vec3(1.0f, 0.0f, 0.0f), 30.0f));
    cube3.AddComponent(new OscillatorComponent(0.5f, 2.0f));

    AppMode previousAppMode = appMode;
    // ================= AUDIO SYSTEM =================

    AudioSystem audioSystem;

    audioSystem.Initialize();

    audioSystem.LoadSound(
        "forest_ambience",
        "Assets/Audio/forest_ambience.wav",
        true
    );
    audioSystem.LoadSound(
        "menu_music",
        "Assets/Audio/menu_music.wav",
        true
    );

    audioSystem.LoadSound(
        "menu_click",
        "Assets/Audio/menu_click.wav",
        false
    );
    audioSystem.LoadSound(
        "interaction",
        "Assets/Audio/interaction.wav",
        false
    );
    audioSystem.LoadSound(
        "forest_ambience",
        "Assets/Audio/forest_ambience.wav",
        true
    );

    audioSystem.LoadSound(
        "interaction",
        "Assets/Audio/interaction.wav",
        false
    );
    audioSystem.LoadSound(
        "walk_grass",
        "Assets/Audio/walk-grass.wav",
        true
    );

    audioSystem.LoadSound(
        "run_grass",
        "Assets/Audio/run-grass.wav",
        true
    );
    audioSystem.LoadSound(
        "coin_collect",
        "Assets/Audio/coin_collect.wav",
        false
    );

    audioSystem.LoadSound(
        "coin_win",
        "Assets/Audio/coin_win.wav",
        false
    );
    audioSystem.LoadSound(
        "coin_lose",
        "Assets/Audio/coin_lose.wav",
        false
    );
   /* audioSystem.Play(
        "forest_ambience",
        0.25f
    );*/
    audioSystem.LoadSound(
        "monster_zone",
        "Assets/Audio/monster_zone.wav",
        false
    );

    audioSystem.LoadSound(
        "monster_chase",
        "Assets/Audio/monster_chase.wav",
        true
    );
    audioSystem.LoadSound(
        "menu_hover",
        "Assets/Audio/menu_hover.wav",
        false
    );
    audioSystem.LoadSound(
        "music_rescue",
        "Assets/Audio/music_rescue.wav",
        true
    );
    // ================= ANIMATION SYSTEM V1A TEST =================

    AnimationLibrary playerAnimationLibrary;

    playerAnimationLibrary.LoadAnimation(
        "Idle",
        "Assets/Models/Charachters/Player/Idle.fbx"
    );

    playerAnimationLibrary.LoadAnimation(
        "Walk",
        "Assets/Models/Charachters/Player/Walk.fbx"
    );

    playerAnimationLibrary.LoadAnimation(
        "Run",
        "Assets/Models/Charachters/Player/Run.fbx"
    );

    playerAnimationLibrary.LoadAnimation(
        "Jump",
        "Assets/Models/Charachters/Player/Jump.fbx"
    );

    playerAnimationLibrary.PrintSummary();
    AnimatedModel* previewAnimatedPlayer =
        playerAnimationLibrary.GetAnimation(
            "Idle"
        );

    bool showAnimatedPlayerPreview =
        true;

    glm::vec3 animatedPreviewPosition =
        glm::vec3(
            0.0f,
            GetTerrainHeight(
                0.0f,
                -8.0f
            ) + 0.05f,
            -8.0f
        );

    float animatedPreviewScale =
        0.018f;
    int animatedPreviewClipIndex =
        0;

    const char* animatedPreviewClipNames[] =
    {
        "Idle",
        "Walk",
        "Run",
        "Jump"
    };

    std::string currentAnimatedPreviewClip =
        "Idle";

    float animatedPreviewRotationY =
        180.0f;

    bool showAnimationPreviewWindow =
        false;

    bool showSceneHealthValidator =
        false;
    bool useAnimatedPlayerVisual =
        false;

    bool hideClassicPlayerWhenAnimated =
        true;

    std::string animatedRuntimeClip =
        "Idle";

    float animatedPlayerScale =
        0.015f;

    float animatedPlayerRotationOffsetY =
        0.0f;

    float animatedPlayerYOffset =
        0.05f;
    glm::vec3 smoothedAnimatedPlayerPosition =
        glm::vec3(
            0.0f
        );

    float smoothedAnimatedPlayerRotationY =
        0.0f;

    bool animatedPlayerSmoothInitialized =
        false;

    float animatedClipSwitchCooldown =
        0.0f;
    bool showAnimatedRuntimeDebug =
        true;

    bool drawAnimationPreviewOnlyInEditor =
        true;
    float animatedJumpLockTimer =
        0.0f;

    float animatedJumpVisualDuration =
        1.5f;

    bool animatedJumpPreviousSpaceDown =
        false;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        cinematicOverlay.HandleShortcut(
            window
        );
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        UpdateResponsiveEditorLayout();
        // ================= RETURN TO MAIN MENU WITH ESC =================
        bool escapeDown =
            glfwGetKey(
                window,
                GLFW_KEY_ESCAPE
            ) == GLFW_PRESS;

        if (
            !showMainMenu &&
            escapeDown &&
            !escapeMenuPressed
            )
        {
            showMainMenu =
                true;

            appMode =
                AppMode::Editor;

            audioSystem.Stop(
                "monster_chase"
            );

            audioSystem.Stop(
                "music_rescue"
            );

            audioSystem.Stop(
                "walk_grass"
            );

            audioSystem.Stop(
                "run_grass"
            );

            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );

            rightMouseCameraActive =
                false;

            firstMouse =
                true;

            ignoreNextMouseDelta =
                true;
        }

        escapeMenuPressed =
            escapeDown;

        // ================= MAIN MENU SCREEN =================
        if (showMainMenu)
        {
            if (!mainMenuMusicPlaying)
            {
                audioSystem.Play(
                    "menu_music",
                    0.45f
                );

                mainMenuMusicPlaying =
                    true;
            }
            MainMenuAction menuAction =
                DrawMainMenuScreen(
                    mainMenuBackgroundTexture
                );
            if (mainMenuLinkClicked)
            {
                audioSystem.PlayFromStart(
                    "menu_click",
                    0.75f
                );

                mainMenuLinkClicked =
                    false;
            }
            if (
                mainMenuHoveredButton != -1 &&
                mainMenuHoveredButton != previousMainMenuHoveredButton
                )
            {
                audioSystem.PlayFromStart(
                    "menu_hover",
                    0.35f
                );
            }

            previousMainMenuHoveredButton =
                mainMenuHoveredButton;
            if (menuAction == MainMenuAction::Play)
            {
                showMainMenu =
                    false;

                appMode =
                    AppMode::Play;

                mainMenuClickPlayed =
                    true;
            }
            else if (menuAction == MainMenuAction::Editor)
            {
                showMainMenu =
                    false;

                appMode =
                    AppMode::Editor;

                mainMenuClickPlayed =
                    true;
            }
            else if (menuAction == MainMenuAction::Exit)
            {
                glfwSetWindowShouldClose(
                    window,
                    true
                );

                mainMenuClickPlayed =
                    true;
            }

            if (mainMenuClickPlayed)
            {
                audioSystem.Stop(
                    "menu_music"
                );

                mainMenuMusicPlaying =
                    false;

                audioSystem.PlayFromStart(
                    "menu_click",
                    1.0f
                );

                if (menuAction != MainMenuAction::Exit)
                {
                    audioSystem.Play(
                        "forest_ambience",
                        0.25f
                    );
                }

                mainMenuClickPlayed =
                    false;
            }

            int menuWidth =
                0;

            int menuHeight =
                0;

            glfwGetFramebufferSize(
                window,
                &menuWidth,
                &menuHeight
            );

            glViewport(
                0,
                0,
                menuWidth,
                menuHeight
            );

            glClearColor(
                0.0f,
                0.0f,
                0.0f,
                1.0f
            );

            glClear(
                GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT
            );
           
            cinematicOverlay.Draw(
                menuWidth,
                menuHeight
            );
            ImGui::Render();

            ImGui_ImplOpenGL3_RenderDrawData(
                ImGui::GetDrawData()
            );

            glfwSwapBuffers(
                window
            );

            continue;
        }
        // ================= LIVE EDIT-TEST WORKFLOW =================

        bool tabModeSwitchDown =
            glfwGetKey(
                window,
                GLFW_KEY_TAB
            ) == GLFW_PRESS;

        if (
            !showMainMenu &&
            tabModeSwitchDown &&
            !tabModeSwitchPressed &&
            !ImGui::GetIO().WantTextInput
            )
        {
            if (appMode == AppMode::Play)
            {
                appMode =
                    AppMode::Editor;

                audioSystem.Stop(
                    "walk_grass"
                );

                audioSystem.Stop(
                    "run_grass"
                );

                audioSystem.Stop(
                    "monster_chase"
                );

                audioSystem.Stop(
                    "music_rescue"
                );

                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_NORMAL
                );

                rightMouseCameraActive =
                    false;

                firstMouse =
                    true;

                ignoreNextMouseDelta =
                    true;

                std::cout
                    << "TAB: Switched from Play Mode to Editor Mode."
                    << std::endl;
            }
            else
            {
                appMode =
                    AppMode::Play;

                glfwSetInputMode(
                    window,
                    GLFW_CURSOR,
                    GLFW_CURSOR_NORMAL
                );

                rightMouseCameraActive =
                    false;

                firstMouse =
                    true;

                ignoreNextMouseDelta =
                    true;

                std::cout
                    << "TAB: Switched from Editor Mode to Play Mode."
                    << std::endl;
            }

            tabModeSwitchPressed =
                true;
        }

        if (!tabModeSwitchDown)
        {
            tabModeSwitchPressed =
                false;
        }
       ImGuiIO& debugIO = ImGui::GetIO();
       // ================= ANIMATION PREVIEW CONTROLS V1C =================
       if (
           appMode == AppMode::Editor &&
           showAnimationPreviewWindow
           )
       {
           ImGui::SetNextWindowPos(
               ImVec2(
                   520.0f,
                   70.0f
               ),
               ImGuiCond_Once
           );

           ImGui::SetNextWindowSize(
               ImVec2(
                   360.0f,
                   330.0f
               
               ),
               ImGuiCond_Once
           );
           ImGui::SetNextWindowPos(
               ImVec2(centerX + 270.0f, topY),
               ImGuiCond_FirstUseEver
           );

           ImGui::SetNextWindowSize(
               ImVec2(340.0f, 350.0f),
               ImGuiCond_FirstUseEver
           );
           ImGui::Begin(
               "Animation Preview"
           );

           ImGui::Text(
               "Animated Character Preview"
           );

           ImGui::Separator();

           if (
               ImGui::Combo(
                   "Animation Clip",
                   &animatedPreviewClipIndex,
                   animatedPreviewClipNames,
                   IM_ARRAYSIZE(
                       animatedPreviewClipNames
                   )
               )
               )
           {
               currentAnimatedPreviewClip =
                   animatedPreviewClipNames[
                       animatedPreviewClipIndex
                   ];

               previewAnimatedPlayer =
                   playerAnimationLibrary.GetAnimation(
                       currentAnimatedPreviewClip
                   );
               if (previewAnimatedPlayer != nullptr)
               {
                   previewAnimatedPlayer->SetRemoveRootMotion(
                       true
                   );
               }
           }

           ImGui::Checkbox(
               "Show Preview",
               &showAnimatedPlayerPreview
           );

           ImGui::DragFloat3(
               "Position",
               &animatedPreviewPosition.x,
               0.10f
           );

           ImGui::DragFloat(
               "Scale",
               &animatedPreviewScale,
               0.001f,
               0.001f,
               1.0f
           );

           ImGui::DragFloat(
               "Rotation Y",
               &animatedPreviewRotationY,
               1.0f,
               -360.0f,
               360.0f
           );

           if (
               ImGui::Button(
                   "Snap Preview To Terrain"
               )
               )
           {
               animatedPreviewPosition.y =
                   GetTerrainHeight(
                       animatedPreviewPosition.x,
                       animatedPreviewPosition.z
                   ) +
                   0.05f;
           }

           ImGui::Separator();

           ImGui::Text(
               "Current Clip: %s",
               currentAnimatedPreviewClip.c_str()
           );
           if (previewAnimatedPlayer != nullptr)
           {
               if (previewAnimatedPlayer->IsPlaying())
               {
                   if (ImGui::Button("Pause Animation"))
                   {
                       previewAnimatedPlayer->Pause();
                   }
               }
               else
               {
                   if (ImGui::Button("Play Animation"))
                   {
                       previewAnimatedPlayer->Play(
                           false
                       );
                   }
               }

               ImGui::SameLine();

               if (ImGui::Button("Restart Animation"))
               {
                   previewAnimatedPlayer->Play(
                       true
                   );
               }

               float animationSpeed =
                   previewAnimatedPlayer->GetAnimationSpeed();

               if (
                   ImGui::DragFloat(
                       "Animation Speed",
                       &animationSpeed,
                       0.05f,
                       0.0f,
                       3.0f
                   )
                   )
               {
                   previewAnimatedPlayer->SetAnimationSpeed(
                       animationSpeed
                   );
               }

               ImGui::Text(
                   "Time: %.2f / %.2f",
                   previewAnimatedPlayer->GetAnimationTime(),
                   previewAnimatedPlayer->GetAnimationDuration()
               );
           }
           if (previewAnimatedPlayer != nullptr)
           {
               ImGui::Text(
                   "Loaded: %s",
                   previewAnimatedPlayer->IsLoaded() ? "Yes" : "No"
               );

               ImGui::Text(
                   "Meshes: %d",
                   previewAnimatedPlayer->GetMeshCount()
               );

               ImGui::Text(
                   "Bones: %d",
                   previewAnimatedPlayer->GetBoneCount()
               );

               ImGui::Text(
                   "Animations: %d",
                   previewAnimatedPlayer->GetAnimationCount()
               );

               ImGui::Text(
                   "Vertices: %d",
                   previewAnimatedPlayer->GetVertexCount()
               );
           }
           else
           {
               ImGui::Text(
                   "No animation clip selected."
               );
           }

           ImGui::End();
       }
       if (
           appMode == AppMode::Editor &&
           showSceneHealthValidator
           )
       {
           DrawSceneHealthValidator(
               scene,
               playerObject,
               useAnimatedPlayerVisual
           );
       }
       // resseting plaaayer to the start .............................
   /*     if (
            previousAppMode == AppMode::Editor &&
            appMode == AppMode::Play
            )
        {
            if (playerObject != nullptr)
            {
                playerObject->transform.position =
                    playerSpawnPosition;

                playerObject->transform.rotation =
                    glm::vec3(0.0f);

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
        }*/
       if (
           previousAppMode == AppMode::Editor &&
           appMode == AppMode::Play
           )
       {
           coinRushGameMode.Reset(
               scene
           );
           monsterEscapeGameMode.ResetForPlay(
               scene
           );
           ResetMusicRescueForPlay(
               scene
           );

           audioSystem.Stop(
               "music_rescue"
           );
           audioSystem.Stop(
               "monster_chase"
           );
         

           mainMenuMusicPlaying =
               false;
     
           if (useAnimatedPlayerVisual)
           {
               animatedRuntimeClip =
                   "Idle";

               animatedClipSwitchCooldown =
                   0.0f;

               animatedJumpLockTimer =
                   0.0f;

               animatedJumpPreviousSpaceDown =
                   false;

               previewAnimatedPlayer =
                   playerAnimationLibrary.GetAnimation(
                       animatedRuntimeClip
                   );

               if (previewAnimatedPlayer != nullptr)
               {
                   ConfigureRuntimeAnimationClip(
                       previewAnimatedPlayer,
                       animatedRuntimeClip
                   );

                   previewAnimatedPlayer->Play(
                       true
                   );
               }

               animatedPlayerSmoothInitialized =
                   false;
           }
       }
       if (
           previousAppMode == AppMode::Play &&
           appMode == AppMode::Editor
           )
       {
           audioSystem.Stop(
               "monster_chase"
           );

           audioSystem.Stop(
               "music_rescue"
           );

           monsterEscapeGameMode.RestoreForEditor(
               scene
           );
           RestoreMusicRescueForEditor(
               scene
           );
       }
        previousAppMode =
            appMode;
       
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        gameplayFeedbackFX.Update(
            deltaTime
        );
        dayNightSystem.Update(
            deltaTime
        );

        weatherSystem.ApplyToAtmosphere(
            dayNightSystem.skyColor,
            dayNightSystem.sunColor
        );
        if (!editorCameraStartFixed)
        {
            camera.Position =
                glm::vec3(
                    0.0f,
                    25.0f,
                    45.0f
                );

            editorCameraStartFixed =
                true;
        }
        float cameraSpeed =
    5.0f *
    deltaTime;

if (
    appMode == AppMode::Editor &&
    glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
)
{
    cameraSpeed =
        15.0f *
        deltaTime;
}
blockEditorMouseLook =
appMode == AppMode::Play;



ImGuiIO& io = ImGui::GetIO();
      
        bool rightMouseDown =
            glfwGetMouseButton(
                window,
                GLFW_MOUSE_BUTTON_RIGHT
            ) == GLFW_PRESS;

        bool canControlCamera =
            rightMouseDown &&
            !io.WantCaptureMouse;

        if (
            canControlCamera &&
            !rightMouseCameraActive
            )
        {
            double mouseX;
            double mouseY;

            glfwGetCursorPos(
                window,
                &mouseX,
                &mouseY
            );

            lastX =
                static_cast<float>(mouseX);

            lastY =
                static_cast<float>(mouseY);

            firstMouse =
                true;

            ignoreNextMouseDelta =
                true;

            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_DISABLED
            );

            rightMouseCameraActive =
                true;
        }
        else if (
            !canControlCamera &&
            rightMouseCameraActive
            )
        {
            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );

            rightMouseCameraActive =
                false;

            firstMouse =
                true;

            ignoreNextMouseDelta =
                true;
        }
        bool nKeyCurrent =
            glfwGetKey(
                window,
                GLFW_KEY_N
            ) == GLFW_PRESS;
        nKeyLastState = nKeyCurrent;

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mKeyPressed)
        {
            if (selectedObject != nullptr)
            {
                for (auto it = scene.objects.begin(); it != scene.objects.end(); ++it)
                {
                    if (*it == selectedObject)
                    {
                        SceneObject* toDelete = *it;
                        scene.objects.erase(it);
                        delete toDelete;
                        selectedObject = nullptr;
                        break;
                    }
                }

                selectedObject = nullptr;

                std::cout << "Selected object deleted!\n";
            }

            mKeyPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        {
            mKeyPressed = false;
        }
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr &&
            !ImGui::GetIO().WantCaptureKeyboard
            )
        {
            float speed =
                5.0f *
                deltaTime;

            bool shiftPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_LEFT_SHIFT
                ) == GLFW_PRESS ||
                glfwGetKey(
                    window,
                    GLFW_KEY_RIGHT_SHIFT
                ) == GLFW_PRESS;

            bool ctrlPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_LEFT_CONTROL
                ) == GLFW_PRESS ||
                glfwGetKey(
                    window,
                    GLFW_KEY_RIGHT_CONTROL
                ) == GLFW_PRESS;

            if (shiftPressed)
                speed *= 4.0f;

            if (ctrlPressed)
                speed *= 0.25f;

            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                selectedObject->transform.position.z -= speed;

            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                selectedObject->transform.position.z += speed;

            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                selectedObject->transform.position.x -= speed;

            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                selectedObject->transform.position.x += speed;

            if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
                selectedObject->transform.position.y += speed;

            if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
                selectedObject->transform.position.y -= speed;

            if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            {
                float terrainOffset =
                    glm::max(
                        selectedObject->transform.scale.y * 0.5f,
                        0.05f
                    );

                selectedObject->transform.position.y =
                    GetObjectTerrainY(
                        selectedObject->transform.position.x,
                        selectedObject->transform.position.z,
                        terrainOffset
                    );
            }
        }
        
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        {
            mKeyPressed = false;
        }
        // ImGuiIO& io = ImGui::GetIO();

        if (appMode == AppMode::Editor)
        {
            if (!io.WantCaptureKeyboard)
            {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    camera.Position += cameraSpeed * camera.Front;

                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    camera.Position -= cameraSpeed * camera.Front;

                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    camera.Position -=
                    glm::normalize(
                        glm::cross(
                            camera.Front,
                            camera.Up
                        )
                    ) * cameraSpeed;

                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    camera.Position +=
                    glm::normalize(
                        glm::cross(
                            camera.Front,
                            camera.Up
                        )
                    ) * cameraSpeed;
            }
        }
        else
        {
            if (!ShouldLockPlayerAfterRuntimeResult())
            {
                thirdPersonController.Update(
                    window,
                    playerObject,
                    camera,
                    scene,
                    deltaTime
                );
            }
            if (
                IsCoinRushModeEnabled() &&
                !monsterEscapeGameMode.playerCaught &&
                !musicRescueWin
                )
            {
                coinRushGameMode.Update(
                    scene,
                    audioSystem,
                    gameplayFeedbackFX,
                    deltaTime
                );
            }
            if (IsMonsterEscapeModeEnabled())
            {
                monsterEscapeGameMode.UpdateTrigger(
                    scene,
                    audioSystem,
                    gameplayFeedbackFX
                );
            }

            if (IsMusicRescueModeEnabled())
            {
                UpdateMusicRescueEvent(
                    scene,
                    audioSystem,
                    deltaTime
                );
            }

            if (IsMonsterEscapeModeEnabled())
            {
                monsterEscapeGameMode.UpdateChase(
                    scene,
                    audioSystem,
                    gameplayFeedbackFX,
                    deltaTime,
                    GetTerrainHeight
                );
            }
        }
        // ================= TERRAIN PLAYER COLLISION =================

        if (appMode == AppMode::Play && playerObject != nullptr)
        {
            float targetY =
                GetPlayerTerrainY(
                    playerObject->transform.position.x,
                    playerObject->transform.position.z
                );

            bool jumpHeld =
                glfwGetKey(
                    window,
                    GLFW_KEY_SPACE
                ) == GLFW_PRESS;

            bool movingUp =
                thirdPersonController.verticalVelocity > 0.01f;

            bool nearGround =
                playerObject->transform.position.y <=
                targetY + 0.65f;

            if (movingUp)
            {
                thirdPersonController.isGrounded =
                    false;
            }

            if (
                !jumpHeld &&
                thirdPersonController.isGrounded
                )
            {
                playerObject->transform.position.y =
                    targetY;

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
            else if (
                !movingUp &&
                nearGround
                )
            {
                playerObject->transform.position.y =
                    targetY;

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
        }
        if (appMode == AppMode::Play && playerObject != nullptr)
        {
            UpdatePlayModeCameraFollow(
                camera,
                playerObject,
                deltaTime
            );
        }
        // ================= FOOTSTEP AUDIO SYSTEM V1 =================

        bool playerMoving =
            false;

        bool playerRunning =
            false;

        if (
            appMode == AppMode::Play &&
            playerObject != nullptr
            )
        {
            bool forwardPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_W
                ) == GLFW_PRESS;

            bool backwardPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_S
                ) == GLFW_PRESS;

            bool leftPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_A
                ) == GLFW_PRESS;

            bool rightPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_D
                ) == GLFW_PRESS;

            bool shiftPressed =
                glfwGetKey(
                    window,
                    GLFW_KEY_LEFT_SHIFT
                ) == GLFW_PRESS ||
                glfwGetKey(
                    window,
                    GLFW_KEY_RIGHT_SHIFT
                ) == GLFW_PRESS;

            playerMoving =
                forwardPressed ||
                backwardPressed ||
                leftPressed ||
                rightPressed;

            playerRunning =
                playerMoving &&
                shiftPressed;
        }

        if (playerMoving)
        {
            if (playerRunning)
            {
                if (!runningFootstepPlaying)
                {
                    audioSystem.Play(
                        "run_grass",
                        0.45f
                    );

                    runningFootstepPlaying =
                        true;
                }

                if (walkingFootstepPlaying)
                {
                    audioSystem.Stop(
                        "walk_grass"
                    );

                    walkingFootstepPlaying =
                        false;
                }
            }
            else
            {
                if (!walkingFootstepPlaying)
                {
                    audioSystem.Play(
                        "walk_grass",
                        0.35f
                    );

                    walkingFootstepPlaying =
                        true;
                }

                if (runningFootstepPlaying)
                {
                    audioSystem.Stop(
                        "run_grass"
                    );

                    runningFootstepPlaying =
                        false;
                }
            }
        }
        else
        {
            if (walkingFootstepPlaying)
            {
                audioSystem.Stop(
                    "walk_grass"
                );

                walkingFootstepPlaying =
                    false;
            }

            if (runningFootstepPlaying)
            {
                audioSystem.Stop(
                    "run_grass"
                );

                runningFootstepPlaying =
                    false;
            }
        }
        // ================= GAMEPLAY INTERACTION SYSTEM V1 =================

        if (interactionResultTimer > 0.0f)
        {
            interactionResultTimer -=
                deltaTime;
        }

        nearbyInteractableObject =
            nullptr;

        interactionHintText =
            "";

        if (
            appMode == AppMode::Play &&
            playerObject != nullptr
            )
        {
            nearbyInteractableObject =
                FindNearestInteractableObject(
                    scene,
                    playerObject->transform.position,
                    interactionRadius
                );

            nearbyInteractableDistance =
                -1.0f;

            if (nearbyInteractableObject != nullptr)
            {
                glm::vec2 playerXZ =
                    glm::vec2(
                        playerObject->transform.position.x,
                        playerObject->transform.position.z
                    );

                glm::vec2 objectXZ =
                    glm::vec2(
                        nearbyInteractableObject->transform.position.x,
                        nearbyInteractableObject->transform.position.z
                    );

                nearbyInteractableDistance =
                    glm::length(
                        playerXZ -
                        objectXZ
                    );
            }

            if (nearbyInteractableObject != nullptr)
            {
                interactionHintText =
                    "Press E to " +
                    GetInteractionActionText(
                        nearbyInteractableObject
                    ) +
                    ": " +
                    nearbyInteractableObject->name;
            }

            bool eKeyDown =
                glfwGetKey(
                    window,
                    GLFW_KEY_E
                ) == GLFW_PRESS;

            if (
                eKeyDown &&
                !interactionKeyPressed &&
                nearbyInteractableObject != nullptr
                )
            {
                interactionCount++;

                if (IsTorchObject(nearbyInteractableObject))
                {
                    ToggleTorchLight(
                        nearbyInteractableObject
                    );
                }

                interactionResultText =
                    GetInteractionResultText(
                        nearbyInteractableObject
                    );

                interactionResultTimer =
                    3.0f;

                std::cout
                    << "Interaction "
                    << interactionCount
                    << ": "
                    << interactionResultText
                    << std::endl;

                audioSystem.PlayFromStart(
                    "interaction",
                    0.85f
                );
            }

            interactionKeyPressed =
                eKeyDown;
        }
        else
        {
            interactionKeyPressed =
                false;
        }

        // ================= GAMEPLAY HUD =================
        // ================= BETTER PLAY MODE HUD =================

        if (appMode == AppMode::Play)
        {
            ImGui::SetNextWindowPos(
                ImVec2(
                    20.0f,
                    90.0f
                ),
                ImGuiCond_FirstUseEver
            );

            ImGui::Begin(
                "ORION Runtime HUD",
                nullptr,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_AlwaysAutoResize
            );

            ImGui::TextColored(
                ImVec4(
                    0.35f,
                    0.75f,
                    1.0f,
                    1.0f
                ),
                "ORION RUNTIME"
            );

            ImGui::Separator();

            ImGui::Text(
                "Mode: %s",
                GetSelectedRuntimeGameModeName()
            );

            ImGui::Spacing();

            ImGui::TextColored(
                ImVec4(
                    1.0f,
                    0.85f,
                    0.25f,
                    1.0f
                ),
                "Objective"
            );

            if (selectedRuntimeGameMode == RuntimeGameMode::FreeRoam)
            {
                ImGui::TextWrapped(
                    "Explore the scene and test editor-created objects."
                );
            }
            else if (selectedRuntimeGameMode == RuntimeGameMode::CoinRush)
            {
                ImGui::TextWrapped(
                    "Collect all coins before the timer ends."
                );
            }
            else if (selectedRuntimeGameMode == RuntimeGameMode::MonsterEscape)
            {
                ImGui::TextWrapped(
                    "Enter the trigger zone and survive the monster chase."
                );
            }
            else
            {
                ImGui::TextWrapped(
                    "Full demo: collect coins, trigger the monster, and complete music rescue."
                );
            }

            ImGui::Separator();

            if (IsCoinRushModeEnabled())
            {
                ImGui::TextColored(
                    ImVec4(
                        1.0f,
                        0.82f,
                        0.20f,
                        1.0f
                    ),
                    "Coin Rush"
                );

                if (coinRushGameMode.active)
                {
                    int timeSeconds =
                        static_cast<int>(
                            coinRushGameMode.timeRemaining
                            );

                    int minutes =
                        timeSeconds / 60;

                    int seconds =
                        timeSeconds % 60;

                    float coinProgress =
                        0.0f;

                    if (coinRushGameMode.totalCount > 0)
                    {
                        coinProgress =
                            static_cast<float>(
                                coinRushGameMode.collectedCount
                                ) /
                            static_cast<float>(
                                coinRushGameMode.totalCount
                                );
                    }

                    ImGui::Text(
                        "Coins: %d / %d",
                        coinRushGameMode.collectedCount,
                        coinRushGameMode.totalCount
                    );

                    ImGui::ProgressBar(
                        coinProgress,
                        ImVec2(
                            260.0f,
                            12.0f
                        )
                    );

                    ImGui::Text(
                        "Time Left: %02d:%02d",
                        minutes,
                        seconds
                    );

                    if (coinRushGameMode.won)
                    {
                        ImGui::TextColored(
                            ImVec4(
                                0.2f,
                                1.0f,
                                0.2f,
                                1.0f
                            ),
                            "State: WIN"
                        );
                    }
                    else if (coinRushGameMode.lost)
                    {
                        ImGui::TextColored(
                            ImVec4(
                                1.0f,
                                0.2f,
                                0.2f,
                                1.0f
                            ),
                            "State: LOSE"
                        );
                    }
                    else
                    {
                        ImGui::TextColored(
                            ImVec4(
                                1.0f,
                                0.85f,
                                0.25f,
                                1.0f
                            ),
                            "State: ACTIVE"
                        );
                    }
                }
                else
                {
                    ImGui::TextDisabled(
                        "No coins placed in the scene."
                    );
                }

                ImGui::Separator();
            }

            if (IsMonsterEscapeModeEnabled())
            {
                ImGui::TextColored(
                    ImVec4(
                        1.0f,
                        0.35f,
                        0.25f,
                        1.0f
                    ),
                    "Monster Escape"
                );

                ImGui::TextWrapped(
                    "%s",
                    monsterEscapeGameMode.eventText.c_str()
                );

                if (monsterEscapeGameMode.playerCaught)
                {
                    ImGui::TextColored(
                        ImVec4(
                            1.0f,
                            0.15f,
                            0.15f,
                            1.0f
                        ),
                        "State: LOSE"
                    );
                }
                else if (monsterEscapeGameMode.active)
                {
                    ImGui::TextColored(
                        ImVec4(
                            1.0f,
                            0.65f,
                            0.15f,
                            1.0f
                        ),
                        "State: CHASING"
                    );
                }
                else
                {
                    ImGui::TextDisabled(
                        "State: WAITING FOR TRIGGER"
                    );
                }

                ImGui::Separator();
            }

            if (IsMusicRescueModeEnabled())
            {
                ImGui::TextColored(
                    ImVec4(
                        0.45f,
                        0.85f,
                        1.0f,
                        1.0f
                    ),
                    "Music Rescue"
                );

                ImGui::TextWrapped(
                    "%s",
                    musicRescueText.c_str()
                );

                if (musicRescueWin)
                {
                    ImGui::TextColored(
                        ImVec4(
                            0.2f,
                            1.0f,
                            0.2f,
                            1.0f
                        ),
                        "State: WIN"
                    );
                }
                else if (musicNpcChasingMonster)
                {
                    ImGui::TextColored(
                        ImVec4(
                            0.45f,
                            0.85f,
                            1.0f,
                            1.0f
                        ),
                        "State: NPC CHASING MONSTER"
                    );
                }
                else if (musicRescueActive)
                {
                    ImGui::TextColored(
                        ImVec4(
                            1.0f,
                            0.85f,
                            0.25f,
                            1.0f
                        ),
                        "State: ACTIVE"
                    );
                }
                else
                {
                    ImGui::TextDisabled(
                        "State: WAITING"
                    );
                }

                ImGui::Separator();
            }

            ImGui::TextColored(
                ImVec4(
                    0.7f,
                    1.0f,
                    0.7f,
                    1.0f
                ),
                "Interaction"
            );

            if (!interactionHintText.empty())
            {
                ImGui::TextWrapped(
                    "%s",
                    interactionHintText.c_str()
                );

                ImGui::Text(
                    "Distance: %.2f / %.2f",
                    nearbyInteractableDistance,
                    interactionRadius
                );
            }
            else
            {
                ImGui::TextDisabled(
                    "No interaction target nearby."
                );
            }

            if (interactionResultTimer > 0.0f)
            {
                ImGui::TextWrapped(
                    "Last Action: %s",
                    interactionResultText.c_str()
                );
            }

            ImGui::Text(
                "Interactions: %d",
                interactionCount
            );

            ImGui::Separator();

            ImGui::TextDisabled(
                "WASD Move | Shift Run | Space Jump | E Interact | TAB Editor/Play"
            );

            ImGui::End();
        }
        // ================= UNDO / REDO SHORTCUTS =================

        if (
            !showMainMenu &&
            appMode == AppMode::Editor
            )
        {
            HandleTransformHistoryShortcuts(
                window,
                scene,
                selectedObject
            );
        }
        if (appMode == AppMode::Play)
        {
            DrawRuntimeResultOverlay();
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            currentAxis = AXIS_X;


        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            currentGizmoMode = TRANSLATE;
            std::cout << "Translate Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            currentGizmoMode = ROTATE;
            std::cout << "Rotate Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        {
            currentGizmoMode = SCALE;
            std::cout << "Scale Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            currentAxis = AXIS_Y;

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            currentAxis = AXIS_Z;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            currentAxis = NONE;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cKeyPressed)
        {
            useCulling = !useCulling;
            cKeyPressed = true;



            if (useCulling)
                std::cout << "Culling ON\n";
            else
                std::cout << "Culling OFF\n";
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed)
        {
            SaveEditorObjects(
                scene,
                "SavedEditorObjects.txt"
            );

            pPressed = true;
         
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        {
            pPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lPressed)
        {
            LoadEditorObjects(
                scene,
                "SavedEditorObjects.txt",
                &cube,
                &shader,
                &cubeMaterial,
                selectedObject,
                &woodenHouseModel,
                &newHouseModel,
                &pineTreeModel,
                &commonTreeModel,
                &rockModel,
                &bushModel,
                &woodLogModel,
                &treeStumpModel,
                &grassModel
            );

            lPressed = true;
            lPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
        {
            lPressed = false;
        }

        static bool gPressed = false;

        /* if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed)
         {
             useGridSnap = !useGridSnap;
             gPressed = true;

             std::cout << "Grid Snap: " << (useGridSnap ? "ON\n" : "OFF\n");
         }

         if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
         {
             gPressed = false;
         }*/

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bPressed)
        {
            if (selectedObject != nullptr)
            {
                SceneObject* copy = new SceneObject(
                    selectedObject->mesh,
                    selectedObject->shader,
                    selectedObject->material
                );


                copy->transform = selectedObject->transform;


                copy->transform.position += glm::vec3(1.0f, 0.0f, 0.0f);

                scene.AddObject(copy);


                selectedObject->isSelected = false;
                selectedObject = copy;
                selectedObject->isSelected = true;

                std::cout << "Object duplicated!\n";
            }

            bPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
        {
            bPressed = false;
        }
        // ===== ROTate
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr
            )
        {
            float rotSpeed = 50.0f * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
                selectedObject->transform.rotation.x += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                selectedObject->transform.rotation.x -= rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
                selectedObject->transform.rotation.y += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
                selectedObject->transform.rotation.y -= rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
                selectedObject->transform.rotation.z += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
                selectedObject->transform.rotation.z -= rotSpeed;
            if (snapEnabled)
            {
                glm::vec3& pos = selectedObject->transform.position;

                pos.x = round(pos.x / gridSize) * gridSize;
                pos.y = round(pos.y / gridSize) * gridSize;
                pos.z = round(pos.z / gridSize) * gridSize;
            }
        }
        //if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        //    currentAxis = X_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        //    currentAxis = Y_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        //    currentAxis = Z_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        //    currentAxis = FREE;// reset
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed)
        {
            snapEnabled = !snapEnabled;
            gPressed = true;

            if (snapEnabled)
                std::cout << "Grid Snapping ON\n";
            else
                std::cout << "Grid Snapping OFF\n";
        }
       
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
        {
            gPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            currentAxis = AXIS_X;
            std::cout << "Move X axis\n";
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            currentAxis = AXIS_Z;
            std::cout << "Move Z axis\n";
        }
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            currentAxis = NONE;
        
        }

        // ===== SCALE
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr
            )
        {
            float scaleSpeed = 2.0f * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) // +
                selectedObject->transform.scale += glm::vec3(scaleSpeed);

            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) // -
                selectedObject->transform.scale -= glm::vec3(scaleSpeed);
            selectedObject->transform.scale =
                glm::max(
                    selectedObject->transform.scale,
                    glm::vec3(0.1f)
                );
        }

        //selectedObject->transform.scale = glm::max(selectedObject->transform.scale, glm::vec3(0.1f));
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        {
            cKeyPressed = false;
        }
        int width, height;

        glfwGetFramebufferSize(
            window,
            &width,
            &height
        );

        glViewport(
            0,
            0,
            width,
            height
        );
        glClearColor(
            dayNightSystem.skyColor.r,
            dayNightSystem.skyColor.g,
            dayNightSystem.skyColor.b,
            1.0f
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
            (float)glfwGetTime(),
            glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)width / (float)height,
            0.1f,
            900.0f
        );
        if (!showMainMenu)
        {
            weatherSystem.DrawOverlay(
                width,
                height
            );
        }
        if (!showMainMenu)
        {
            objectiveMarkerSystem.Draw(
                scene,
                camera.Position,
                view,
                projection,
                width,
                height
            );
        }
        if (!showMainMenu)
        {
            gameplayFeedbackFX.Draw(
                view,
                projection,
                width,
                height,
                monsterEscapeGameMode.active,
                monsterEscapeGameMode.playerCaught,
                musicRescueWin
            );
        }
        if (!showMainMenu)
        {
            miniMapRadar.Draw(
                scene,
                width,
                height
            );
        }
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        gridShader.use();

        gridShader.setMat4("view", glm::value_ptr(view));
        gridShader.setMat4("projection", glm::value_ptr(projection));

        grid.Draw(gridShader);
        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", glm::value_ptr(skyboxView));
        skyboxShader.setMat4("projection", glm::value_ptr(projection));
        glm::mat4 vp = projection * view;
        frustum.Update(vp);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        skybox.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        if (
            appMode == AppMode::Editor &&
            mouseClicked &&
            !ImGui::GetIO().WantCaptureMouse
            )
        {
            double mouseX;
            double mouseY;

            glfwGetCursorPos(
                window,
                &mouseX,
                &mouseY
            );

            glm::vec3 rayDir =
                GetRayFromMouse(
                    mouseX,
                    mouseY,
                    width,
                    height,
                    projection,
                    view
                );

            glm::vec3 rayOrigin =
                camera.Position;

            float closestDist =
                1000.0f;

            SceneObject* hitObject =
                nullptr;
            
            for (SceneObject* obj : scene.objects)
            {
                if (!IsEditorSelectableObject(obj))
                    continue;

                float radius =
                    GetEditorSelectionRadius(
                        obj
                    );

                glm::vec3 oc =
                    rayOrigin -
                    obj->transform.position;

                float a =
                    glm::dot(
                        rayDir,
                        rayDir
                    );

                float b =
                    2.0f *
                    glm::dot(
                        oc,
                        rayDir
                    );

                float c =
                    glm::dot(
                        oc,
                        oc
                    ) -
                    radius * radius;

                float discriminant =
                    b * b -
                    4.0f * a * c;

                if (discriminant > 0.0f)
                {
                    float dist =
                        (
                            -b -
                            std::sqrt(
                                discriminant
                            )
                            ) /
                        (
                            2.0f * a
                            );

                    if (
                        dist > 0.0f &&
                        dist < closestDist
                        )
                    {
                        closestDist =
                            dist;

                        hitObject =
                            obj;
                    }
                }
            }

            if (hitObject != nullptr)
            {
                SelectEditorObject(
                    selectedObject,
                    hitObject
                );
            }

            mouseClicked =
                false;
        }
        if (
            appMode == AppMode::Editor &&
            isDragging &&
            selectedObject != nullptr
            )
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // ================= TRANSLATE MODE =================
            if (currentGizmoMode == TRANSLATE)
            {
                glm::vec3 rayDir = GetRayFromMouse(
                    mouseX,
                    mouseY,
                    width,
                    height,
                    projection,
                    view
                );
                glm::vec3 rayOrigin = camera.Position;

                float planeY = selectedObject->transform.position.y;

                float t = (planeY - rayOrigin.y) / rayDir.y;

                if (t > 0.0f)
                {
                    glm::vec3 hitPoint = rayOrigin + rayDir * t;

                    if (currentAxis == AXIS_X)
                        selectedObject->transform.position.x = hitPoint.x;

                    else if (currentAxis == AXIS_Z)
                        selectedObject->transform.position.z = hitPoint.z;

                    else if (currentAxis == AXIS_Y)
                        selectedObject->transform.position.y = hitPoint.y;

                    else
                    {
                        selectedObject->transform.position.x = hitPoint.x;
                        selectedObject->transform.position.z = hitPoint.z;
                    }
                }
            }


            else if (currentGizmoMode == ROTATE)
            {
                float rotationSpeed = 0.2f;

                selectedObject->transform.rotation.y +=
                    (float)(mouseX - lastX) * rotationSpeed;
            }


            else if (currentGizmoMode == SCALE)
            {
                float scaleSpeed = 0.01f;

                float scaleAmount =
                    (float)(mouseX - lastX) * scaleSpeed;

                selectedObject->transform.scale +=
                    glm::vec3(scaleAmount);

                selectedObject->transform.scale =
                    glm::max(
                        selectedObject->transform.scale,
                        glm::vec3(0.1f)
                    );
            }
        }

        shader.use();
        shader.setInt( "texture1",0);
        shader.setInt(
            "terrainGrassTex",
            1
        );

        shader.setInt(
            "terrainDirtTex",
            2
        );

        shader.setInt(
            "terrainCliffTex",
            3
        );

        shader.setInt(
            "terrainGrassTex",
            1
        );

        shader.setInt(
            "terrainDirtTex",
            2
        );

        shader.setInt(
            "terrainCliffTex",
            3
        );
        shader.setBool(
            "isProceduralTerrain",
            false
        );
        shader.setVec3(
            "sunDirection",
            dayNightSystem.sunDirection
        );

        shader.setVec3(
            "sunColor",
            dayNightSystem.sunColor
        );
        shader.setBool(
            "useAtmosphereFog",
            weatherSystem.useFog
        );

        shader.setVec3(
            "atmosphereFogColor",
            weatherSystem.fogColor
        );

        shader.setFloat(
            "atmosphereFogStart",
            weatherSystem.fogStart
        );

        shader.setFloat(
            "atmosphereFogEnd",
            weatherSystem.fogEnd
        );

        shader.setFloat(
            "atmosphereFogStrength",
            weatherSystem.fogStrength
        );
        UpdateTorchFireFlicker(
            scene,
            static_cast<float>(
                glfwGetTime()
                )
        );
        int pointLightIndex =
            0;

        for (Light* light : scene.lights)
        {
            if (light == nullptr)
                continue;

            if (light->type == LightType::Directional)
                continue;

            if (pointLightIndex >= 12)
                break;

            shader.setVec3(
                "lightPositions[" + std::to_string(pointLightIndex) + "]",
                light->position
            );

            shader.setVec3(
                "lightColors[" + std::to_string(pointLightIndex) + "]",
                light->color
            );

            pointLightIndex++;
        }

        for (int i = pointLightIndex; i < 12; i++)
        {
            shader.setVec3(
                "lightPositions[" + std::to_string(i) + "]",
                glm::vec3(
                    0.0f,
                    -1000.0f,
                    0.0f
                )
            );

            shader.setVec3(
                "lightColors[" + std::to_string(i) + "]",
                glm::vec3(
                    0.0f
                )
            );
        }
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("texture1", 0);

        // ===== DRAW CUBES (container.jpg) =====
        glActiveTexture(GL_TEXTURE0);
        containerTexture.Bind();

        // Update all components (rotation, oscillation, etc.)
        cube1.transform.position = glm::vec3(0.0f, 0.0f, -3.0f);
        cube1.UpdateComponents(deltaTime);
        cube1.Draw(renderer, glm::mat4(1.0f));
        // ================= ANIMATED PLAYER RUNTIME STATE V3 CLEANUP =================

        if (playerObject != nullptr)
        {
            playerObject->visible =
                !(
                    useAnimatedPlayerVisual &&
                    hideClassicPlayerWhenAnimated
                    );
        }

        bool spaceDownForAnimatedJump =
            glfwGetKey(
                window,
                GLFW_KEY_SPACE
            ) == GLFW_PRESS;

        bool animatedJumpJustPressed =
            spaceDownForAnimatedJump &&
            !animatedJumpPreviousSpaceDown;

        animatedJumpPreviousSpaceDown =
            spaceDownForAnimatedJump;

        if (
            appMode != AppMode::Play ||
            !useAnimatedPlayerVisual
            )
        {
            animatedJumpLockTimer =
                0.0f;
        }

        if (
            useAnimatedPlayerVisual &&
            playerObject != nullptr
            )
        {
            bool moveForward =
                glfwGetKey(
                    window,
                    GLFW_KEY_W
                ) == GLFW_PRESS;

            bool moveBackward =
                glfwGetKey(
                    window,
                    GLFW_KEY_S
                ) == GLFW_PRESS;

            bool moveLeft =
                glfwGetKey(
                    window,
                    GLFW_KEY_A
                ) == GLFW_PRESS;

            bool moveRight =
                glfwGetKey(
                    window,
                    GLFW_KEY_D
                ) == GLFW_PRESS;

            bool running =
                glfwGetKey(
                    window,
                    GLFW_KEY_LEFT_SHIFT
                ) == GLFW_PRESS;

            bool moving =
                moveForward ||
                moveBackward ||
                moveLeft ||
                moveRight;

            if (
                appMode == AppMode::Play &&
                animatedJumpJustPressed
                )
            {
                animatedJumpLockTimer =
                    animatedJumpVisualDuration;
            }

            if (animatedJumpLockTimer > 0.0f)
            {
                animatedJumpLockTimer -=
                    deltaTime;

                if (animatedJumpLockTimer < 0.0f)
                {
                    animatedJumpLockTimer =
                        0.0f;
                }
            }

            std::string wantedClip =
                "Idle";

            if (
                appMode == AppMode::Play &&
                animatedJumpLockTimer > 0.0f
                )
            {
                wantedClip =
                    "Jump";
            }
            else if (
                appMode == AppMode::Play &&
                moving &&
                running
                )
            {
                wantedClip =
                    "Run";
            }
            else if (
                appMode == AppMode::Play &&
                moving
                )
            {
                wantedClip =
                    "Walk";
            }
            else
            {
                wantedClip =
                    "Idle";
            }

            animatedClipSwitchCooldown -=
                deltaTime;

            if (animatedClipSwitchCooldown < 0.0f)
            {
                animatedClipSwitchCooldown =
                    0.0f;
            }

            bool canSwitchClip =
                animatedClipSwitchCooldown <= 0.0f;

            // Jump must start immediately, even if cooldown is active.
            if (wantedClip == "Jump")
            {
                canSwitchClip =
                    true;
            }

            if (
                wantedClip != animatedRuntimeClip &&
                canSwitchClip
                )
            {
                animatedRuntimeClip =
                    wantedClip;

                previewAnimatedPlayer =
                    playerAnimationLibrary.GetAnimation(
                        animatedRuntimeClip
                    );

                if (previewAnimatedPlayer != nullptr)
                {
                    ConfigureRuntimeAnimationClip(
                        previewAnimatedPlayer,
                        animatedRuntimeClip
                    );

                    previewAnimatedPlayer->Play(
                        true
                    );
                }

                if (animatedRuntimeClip == "Jump")
                {
                    animatedClipSwitchCooldown =
                        animatedJumpVisualDuration;
                }
                else
                {
                    animatedClipSwitchCooldown =
                        0.18f;
                }
            }
        }

        if (previewAnimatedPlayer != nullptr)
        {
            previewAnimatedPlayer->UpdateAnimation(
                deltaTime
            );
        }
        int totalObjects =
            static_cast<int>(
                scene.objects.size()
                );

        int visibleObjects =
            0;

        int culledObjects =
            0;

        distanceCulledObjects =
            0;

        glm::vec3 performanceViewerPosition =
            camera.Position;

        if (
            appMode == AppMode::Play &&
            playerObject != nullptr
            )
        {
            performanceViewerPosition =
                playerObject->transform.position;
        }

        for (SceneObject* obj : scene.objects)
        {
            if (obj == nullptr)
                continue;

            if (
                ShouldSkipObjectByDistance(
                    obj,
                    performanceViewerPosition
                )
                )
            {
                distanceCulledObjects++;
                culledObjects++;
                continue;
            }

            obj->UpdateComponents(deltaTime);
            bool objectShouldHighlight =
                obj == selectedObject;

            if (
                appMode == AppMode::Play &&
                obj == nearbyInteractableObject
                )
            {
                objectShouldHighlight =
                    true;
            }

            shader.setBool(
                "isSelected",
                objectShouldHighlight
            );

            if (objectShouldHighlight)
            {
                glPolygonMode(
                    GL_FRONT_AND_BACK,
                    GL_LINE
                );

                glLineWidth(
                    3.0f
                );
            }
            else
            {
                glPolygonMode(
                    GL_FRONT_AND_BACK,
                    GL_FILL
                );
            }
            if (
                !useCulling ||
                frustum.IsSphereVisible(
                    obj->transform.position,
                    obj->boundingRadius
                )
                )
            {
                visibleObjects++;

                bool objectIsProceduralTerrain =
                    obj->name == "Procedural Terrain";

                shader.setBool(
                    "isProceduralTerrain",
                    objectIsProceduralTerrain
                );
                if (objectIsProceduralTerrain)
                {
                    glActiveTexture(GL_TEXTURE1);
                    terrainGrassTexture.Bind();

                    glActiveTexture(GL_TEXTURE2);
                    terrainDirtTexture.Bind();

                    glActiveTexture(GL_TEXTURE3);
                    terrainCliffTexture.Bind();

                    shader.setBool(
                        "useTexture",
                        false
                    );

                    shader.setVec3(
                        "materialAmbient",
                        obj->material->ambient
                    );

                    shader.setVec3(
                        "materialDiffuse",
                        obj->material->diffuse
                    );

                    shader.setVec3(
                        "materialSpecular",
                        obj->material->specular
                    );

                    shader.setVec3(
                        "materialTint",
                        obj->material->tint
                    );

                    shader.setFloat(
                        "materialShininess",
                        obj->material->shininess
                    );

                    glPolygonMode(
                        GL_FRONT_AND_BACK,
                        GL_FILL
                    );
                }
                else if (obj->material != nullptr)
                {
                    bool objectHasTexture =
                        obj->material->texture != nullptr;

                    shader.setBool(
                        "useTexture",
                        objectHasTexture
                    );

                    if (objectHasTexture)
                    {
                        glActiveTexture(GL_TEXTURE0);

                        obj->material->texture->Bind();
                    }

                    shader.setVec3("materialAmbient", obj->material->ambient);
                    shader.setVec3("materialDiffuse", obj->material->diffuse);
                    shader.setVec3("materialSpecular", obj->material->specular);
                    shader.setVec3("materialTint", obj->material->tint);
                    shader.setFloat("materialShininess", obj->material->shininess);
                    if (obj->material->wireframe)
                    {
                        glPolygonMode(
                            GL_FRONT_AND_BACK,
                            GL_LINE
                        );
                    }
                    else
                    {
                        glPolygonMode(
                            GL_FRONT_AND_BACK,
                            GL_FILL
                        );
                    }
                }
                else
                {
                    shader.setBool(
                        "isProceduralTerrain",
                        false
                    );

                    shader.setBool(
                        "useTexture",
                        obj->useModel
                    );

                    glPolygonMode(
                        GL_FRONT_AND_BACK,
                        GL_FILL
                    );

                    shader.setVec3(
                        "materialAmbient",
                        glm::vec3(
                            0.65f,
                            0.65f,
                            0.65f
                        )
                    );

                    shader.setVec3(
                        "materialDiffuse",
                        glm::vec3(
                            1.0f,
                            1.0f,
                            1.0f
                        )
                    );

                    shader.setVec3(
                        "materialSpecular",
                        glm::vec3(
                            0.03f,
                            0.03f,
                            0.03f
                        )
                    );

                    shader.setVec3(
                        "materialTint",
                        glm::vec3(
                            1.0f,
                            1.0f,
                            1.0f
                        )
                    );

                    shader.setFloat(
                        "materialShininess",
                        4.0f
                    );
                }
                obj->Draw(renderer, glm::mat4(1.0f));
                shader.setBool(
                    "isSelected",
                    false
                );

                glPolygonMode(
                    GL_FRONT_AND_BACK,
                    GL_FILL
                );
            }
            else
            {
                culledObjects++;
            }
        }
        // ================= DRAW ANIMATED PLAYER STATIC PREVIEW V1B =================
        bool shouldDrawAnimatedCharacter =
            false;

        if (
            appMode == AppMode::Editor &&
            showAnimatedPlayerPreview
            )
        {
            shouldDrawAnimatedCharacter =
                true;
        }

        if (
            appMode == AppMode::Play &&
            useAnimatedPlayerVisual
            )
        {
            shouldDrawAnimatedCharacter =
                true;
        }

        if (
            shouldDrawAnimatedCharacter &&
            previewAnimatedPlayer != nullptr &&
            previewAnimatedPlayer->HasPreviewMesh()
            )
        {
            shader.use();

            glm::mat4 animatedModelMatrix =
                glm::mat4(
                    1.0f
                );

            glm::vec3 finalAnimatedPosition =
                animatedPreviewPosition;

            float finalAnimatedScale =
                animatedPreviewScale;

            float finalAnimatedRotationY =
                animatedPreviewRotationY;

            if (
                useAnimatedPlayerVisual &&
                playerObject != nullptr
                )
            {
                glm::vec3 targetAnimatedPosition =
                    playerObject->transform.position;

                targetAnimatedPosition.y +=
                    animatedPlayerYOffset;

                float targetAnimatedRotationY =
                    playerObject->transform.rotation.y +
                    animatedPlayerRotationOffsetY;

                if (!animatedPlayerSmoothInitialized)
                {
                    smoothedAnimatedPlayerPosition =
                        targetAnimatedPosition;

                    smoothedAnimatedPlayerRotationY =
                        targetAnimatedRotationY;

                    animatedPlayerSmoothInitialized =
                        true;
                }

                float positionSmoothFactor =
                    glm::clamp(
                        deltaTime * 12.0f,
                        0.0f,
                        1.0f
                    );

                float rotationSmoothFactor =
                    glm::clamp(
                        deltaTime * 10.0f,
                        0.0f,
                        1.0f
                    );

                smoothedAnimatedPlayerPosition =
                    glm::mix(
                        smoothedAnimatedPlayerPosition,
                        targetAnimatedPosition,
                        positionSmoothFactor
                    );

                smoothedAnimatedPlayerRotationY =
                    LerpAngleDegrees(
                        smoothedAnimatedPlayerRotationY,
                        targetAnimatedRotationY,
                        rotationSmoothFactor
                    );

                finalAnimatedPosition =
                    smoothedAnimatedPlayerPosition;

                finalAnimatedScale =
                    animatedPlayerScale;

                finalAnimatedRotationY =
                    smoothedAnimatedPlayerRotationY;
            }
            animatedModelMatrix =
                glm::translate(
                    animatedModelMatrix,
                    finalAnimatedPosition
                );
            animatedModelMatrix =
                glm::rotate(
                    animatedModelMatrix,
                    glm::radians(
                        finalAnimatedRotationY
                    ),
                    glm::vec3(
                        0.0f,
                        1.0f,
                        0.0f
                    )
                );
            animatedModelMatrix =
                glm::scale(
                    animatedModelMatrix,
                    glm::vec3(
                        finalAnimatedScale
                    )
                );

            shader.setMat4(
                "model",
                glm::value_ptr(
                    animatedModelMatrix
                )
            );

            shader.setBool(
                "useTexture",
                false
            );

            shader.setBool(
                "isTerrain",
                false
            );

            shader.setBool(
                "isSelected",
                false
            );

            shader.setVec3(
                "materialTint",
                glm::vec3(
                    0.75f,
                    0.80f,
                    0.95f
                )
            );

            shader.setVec3(
                "materialAmbient",
                glm::vec3(
                    0.25f,
                    0.25f,
                    0.30f
                )
            );

            shader.setVec3(
                "materialDiffuse",
                glm::vec3(
                    0.75f,
                    0.80f,
                    0.95f
                )
            );

            shader.setVec3(
                "materialSpecular",
                glm::vec3(
                    0.08f,
                    0.08f,
                    0.10f
                )
            );

            shader.setFloat(
                "materialShininess",
                16.0f
            );

            previewAnimatedPlayer->DrawStaticPreview(
                shader
            );
        }
        // ===== DRAW MODELS (each binds its own texture) =====
  /*      glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-3.0f, 2.0f, -3.0f));
        shader.setMat4("model", glm::value_ptr(model1));
        myModel.Draw(shader);*/

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(3.0f, 0.0f, -5.0f));
        shader.setMat4("model", glm::value_ptr(model2));
        treeModel.Draw(shader);

        // ===== DRAW LIGHT CUBE =====
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, glm::vec3(2.0f, 2.0f, 2.0f));
        lightModel = glm::scale(lightModel, glm::vec3(0.1f));
        lightShader.setMat4("model", glm::value_ptr(lightModel));
        lightShader.setMat4("view", glm::value_ptr(view));
        lightShader.setMat4("projection", glm::value_ptr(projection));
        renderer.DrawMesh(cube, lightShader, lightModel);

        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0)
        {
            double fps = frameCount / (currentTime - previousTime);
            std::string title =
                "FPS: " +
                std::to_string(
                    (int)fps
                ) +
                " | Visible: " +
                std::to_string(
                    visibleObjects
                ) +
                " | Culled: " +
                std::to_string(
                    culledObjects
                ) +
                " | Distance Culled: " +
                std::to_string(
                    distanceCulledObjects
                ) +
                " | Total: " +
                std::to_string(
                    totalObjects
                );
            if (selectedObject != nullptr)
            {
                title += " | Selected Pos: (" +
                    std::to_string(selectedObject->transform.position.x) + "," +
                    std::to_string(selectedObject->transform.position.y) + "," +
                    std::to_string(selectedObject->transform.position.z) + ")";
            }
            else
            {
                title += " | No Object Selected";
            }
            if (selectedObject != nullptr)
            {
                glm::vec3 p = selectedObject->transform.position;

                title += " | Selected Pos: (" +
                    std::to_string((int)p.x) + "," +
                    std::to_string((int)p.y) + "," +
                    std::to_string((int)p.z) + ")";
            }
            if (selectedObject != nullptr)
            {
                glm::vec3 p = selectedObject->transform.position;
                glm::vec3 r = selectedObject->transform.rotation;
                glm::vec3 s = selectedObject->transform.scale;

                title += " | Pos(" +
                    std::to_string((int)p.x) + "," +
                    std::to_string((int)p.y) + "," +
                    std::to_string((int)p.z) + ")";

                title += " Rot(" +
                    std::to_string((int)r.x) + "," +
                    std::to_string((int)r.y) + "," +
                    std::to_string((int)r.z) + ")";

                title += " Scale(" +
                    std::to_string((int)s.x) + ")";
            }
            std::string modeText = "Translate";

            if (currentGizmoMode == ROTATE)
                modeText = "Rotate";

            if (currentGizmoMode == SCALE)
                modeText = "Scale";

            title += " | Mode: " + modeText;
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            previousTime = currentTime;

        }
        EditorUI::DrawToolbar(
            scene,
            selectedObject,
            selectedLight,
            &cube,
            &shader,
            &cubeMaterial,
            camera,
            &torchModel,
            lightCounter,
            appMode,

            [&]()
            {
                SaveEditorObjects(
                    scene,
                    "SavedEditorObjects.txt"
                );
            },

            [&]()
            {
                LoadEditorObjects(
                    scene,
                    "SavedEditorObjects.txt",
                    &cube,
                    &shader,
                    &cubeMaterial,
                    selectedObject,
                    &woodenHouseModel,
                    &newHouseModel,
                    &pineTreeModel,
                    &commonTreeModel,
                    &rockModel,
                    &bushModel,
                    &woodLogModel,
                    &treeStumpModel,
                    &grassModel
                );
            }
        );
        if (appMode == AppMode::Editor)
        {
            if (showUIPanelsWindow)
            {
                ImGui::SetNextWindowPos(
                    ImVec2(
                        leftX,
                        topY + 380.0f
                    ),
                    ImGuiCond_FirstUseEver
                );

                ImGui::SetNextWindowSize(
                    ImVec2(
                        leftWidth,
                        280.0f
                    ),
                    ImGuiCond_FirstUseEver
                );

                ImGui::Begin(
                    "UI Panels",
                    nullptr,
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize
                );

                ImGui::Text(
                    "Editor Window Visibility"
                );

                ImGui::Separator();

                if (ImGui::Button("Clean Layout"))
                {
                    showHierarchyPanel =
                        true;

                    showInspectorPanel =
                        true;

                    showAssetBrowserPanel =
                        true;

                    showStatisticsPanel =
                        true;

                    showLightInspectorPanel =
                        false;

                    showDebugPanel =
                        false;

                    showAnimationPreviewWindow =
                        false;

                    showSceneHealthValidator =
                        false;

                    showSelectedObjectToolsPanel =
                        false;
                    showPlayerToolsPanel =
                        true;

                    showVisualPolishPanel =
                        true;
                    showCollisionDebug =
                        false;
                }

                ImGui::SameLine();

                if (ImGui::Button("Show All"))
                {
                    showHierarchyPanel =
                        true;

                    showInspectorPanel =
                        true;

                    showAssetBrowserPanel =
                        true;

                    showStatisticsPanel =
                        true;

                    showLightInspectorPanel =
                        true;

                    showDebugPanel =
                        true;

                    showAnimationPreviewWindow =
                        true;

                    showSceneHealthValidator =
                        true;

                    showSelectedObjectToolsPanel =
                        true;
                    showPlayerToolsPanel =
                        true;

                    showVisualPolishPanel =
                        true;
                    showCollisionDebug =
                        true;
                }
                ImGui::Separator();

                ImGui::Text(
                    "Transform History"
                );

                if (ImGui::Button("Undo"))
                {
                    UndoTransformChange(
                        scene,
                        selectedObject
                    );
                }

                ImGui::SameLine();

                if (ImGui::Button("Redo"))
                {
                    RedoTransformChange(
                        scene,
                        selectedObject
                    );
                }

                ImGui::Text(
                    "Undo: %d | Redo: %d",
                    static_cast<int>(
                        undoTransformStack.size()
                        ),
                    static_cast<int>(
                        redoTransformStack.size()
                        )
                );

                ImGui::TextDisabled(
                    "CTRL+Z / CTRL+Y"
                );
                ImGui::Separator();

                ImGui::Checkbox(
                    "Hierarchy",
                    &showHierarchyPanel
                );

                ImGui::Checkbox(
                    "Inspector",
                    &showInspectorPanel
                );

                ImGui::Checkbox(
                    "Asset Browser",
                    &showAssetBrowserPanel
                );

                ImGui::Checkbox(
                    "Statistics",
                    &showStatisticsPanel
                );

                ImGui::Checkbox(
                    "Light Inspector",
                    &showLightInspectorPanel
                );
                ImGui::Checkbox(
                    "Player Tools",
                    &showPlayerToolsPanel
                );

                ImGui::Checkbox(
                    "Visual Polish",
                    &showVisualPolishPanel
                );
                ImGui::Checkbox(
                    "Debug",
                    &showDebugPanel
                );

                ImGui::Checkbox(
                    "Animation Preview",
                    &showAnimationPreviewWindow
                );
                ImGui::Checkbox(
                    "Playability Validator",
                    &showSceneHealthValidator
                );
                ImGui::Checkbox(
                    "Selected Tools",
                    &showSelectedObjectToolsPanel
                );
                ImGui::Checkbox(
                    "Collision Debug",
                    &showCollisionDebug
                );

                ImGui::End();
            }
            if (showHierarchyPanel)
            {
                EditorUI::DrawHierarchy(
                    scene,
                    selectedObject,
                    selectedLight,
                    &shader,
                    camera
                );
            }

            if (showLightInspectorPanel)
            {
                EditorUI::DrawLightInspector(
                    selectedLight
                );
            }

            if (showInspectorPanel)
            {
                EditorUI::DrawInspector(
                    selectedObject
                );
            }

            if (showDebugPanel)
            {
                EditorUI::DrawDebug(
                    deltaTime,
                    totalObjects,
                    visibleObjects,
                    culledObjects,
                    selectedObject
                );
            }

            if (showStatisticsPanel)
            {
                EditorUI::DrawStatistics(
                    scene,
                    camera,
                    selectedObject,
                    deltaTime
                );
            }
            auto GetEditorForwardXZ =
                [&]()
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

                    return glm::normalize(forward);
                };

            auto SpawnHouseFromAssetBrowser =
                [&](bool useHouse2)
                {
                    glm::vec3 forward =
                        GetEditorForwardXZ();

                    glm::vec3 position =
                        camera.Position +
                        forward * 22.0f;

                    Model* selectedHouse =
                        useHouse2
                        ? &newHouseModel
                        : &woodenHouseModel;

                    glm::vec3 selectedScale =
                        useHouse2
                        ? glm::vec3(0.01f)
                        : glm::vec3(0.8f);

                    float terrainOffset =
                        useHouse2
                        ? -0.65f
                        : 0.05f;

                    std::string houseName =
                        useHouse2
                        ? "House 2 "
                        : "House 1 ";

                    std::string assetId =
                        useHouse2
                        ? "house_2"
                        : "wooden_house";

                    SceneObject* house =
                        AddObjectOnTerrain(
                            selectedHouse,
                            houseName + std::to_string(manualHouseCounter++),
                            position.x,
                            position.z,
                            selectedScale,
                            true,
                            AssetType::House,
                            assetId,
                            true,
                            terrainOffset
                        );

                    house->transform.rotation.y =
                        glm::degrees(
                            std::atan2(
                                forward.x,
                                forward.z
                            )
                        );

                    house->boundingRadius =
                        120.0f;

                    house->colliderRadius =
                        7.0f;

                    selectedObject =
                        house;

                    selectedLight =
                        nullptr;
                };

            auto BuildCampFromAssetBrowser =
                [&](bool useHouse2)
                {
                    glm::vec3 forward =
                        GetEditorForwardXZ();

                    glm::vec3 position =
                        camera.Position +
                        forward * 35.0f;

                    BuildCamp(
                        position.x,
                        position.z,
                        useHouse2
                    );
                };

            auto BuildForestFromAssetBrowser =
                [&]()
                {
                    glm::vec3 forward =
                        GetEditorForwardXZ();

                    glm::vec3 position =
                        camera.Position +
                        forward * 40.0f;

                    BuildForestZone(
                        position.x,
                        position.z,
                        35.0f,
                        28
                    );
                };
            if (showAssetBrowserPanel)
            {
            EditorUI::DrawAssetBrowser(
                scene,
                selectedObject,
                &cube,
                &shader,
                &cubeMaterial,
                camera,

                & woodenHouseModel,
                & newHouseModel,

                & pineTreeModel,
                & commonTreeModel,
                & rockModel,
                & bushModel,
                & woodLogModel,
                & treeStumpModel,
                & grassModel,

                SpawnHouseFromAssetBrowser,
                BuildCampFromAssetBrowser,
                BuildForestFromAssetBrowser
            );
            }
            // ================= PLAYER SPAWN TOOLS =================
            if (showPlayerToolsPanel)
            {
                ImGui::SetNextWindowPos(
                    ImVec2(
                        1240.0f,
                        360.0f
                    ),
                    ImGuiCond_Once
                );

                ImGui::SetNextWindowSize(
                    ImVec2(
                        390.0f,
                        430.0f
                    ),
                    ImGuiCond_Once
                );
                ImGui::SetNextWindowPos(
                    ImVec2(rightX, topY + 420.0f),
                    ImGuiCond_FirstUseEver
                );

                ImGui::SetNextWindowSize(
                    ImVec2(rightWidth, 280.0f),
                    ImGuiCond_FirstUseEver
                );
                ImGui::Begin("Player Tools");



                if (playerObject != nullptr)
                {
                    ImGui::Text(
                        "Character Visual Mode"
                    );

                    bool previousAnimatedVisualState =
                        useAnimatedPlayerVisual;

                    ImGui::Checkbox(
                        "Use Animated Player",
                        &useAnimatedPlayerVisual
                    );

                    if (previousAnimatedVisualState != useAnimatedPlayerVisual)
                    {
                        animatedPlayerSmoothInitialized =
                            false;
                    }

                    ImGui::Checkbox(
                        "Hide Classic Player Mesh",
                        &hideClassicPlayerWhenAnimated
                    );

                    ImGui::DragFloat(
                        "Animated Scale",
                        &animatedPlayerScale,
                        0.001f,
                        0.001f,
                        1.0f
                    );

                    ImGui::DragFloat(
                        "Animated Rotation Offset Y",
                        &animatedPlayerRotationOffsetY,
                        1.0f,
                        -360.0f,
                        360.0f
                    );

                    ImGui::DragFloat(
                        "Animated Y Offset",
                        &animatedPlayerYOffset,
                        0.01f,
                        -5.0f,
                        5.0f
                    );

                    ImGui::Text(
                        "Runtime Animation: %s",
                        animatedRuntimeClip.c_str()
                    );
                    ImGui::Separator();

                    ImGui::Text(
                        "Animation Runtime Settings"
                    );

                    ImGui::Text(
                        "Current Clip: %s",
                        animatedRuntimeClip.c_str()
                    );

                    ImGui::DragFloat(
                        "Jump Visual Duration",
                        &animatedJumpVisualDuration,
                        0.01f,
                        0.30f,
                        1.50f
                    );

                    ImGui::Checkbox(
                        "Show Animation Debug",
                        &showAnimatedRuntimeDebug
                    );

                    if (
                        ImGui::Button(
                            "Reset Animated Player Defaults"
                        )
                        )
                    {
                        animatedPlayerScale =
                            0.015f;

                        animatedPlayerRotationOffsetY =
                            0.0f;

                        animatedPlayerYOffset =
                            0.05f;

                        animatedJumpVisualDuration =
                            0.85f;

                        animatedPlayerSmoothInitialized =
                            false;

                        animatedClipSwitchCooldown =
                            0.0f;

                        animatedJumpLockTimer =
                            0.0f;
                    }
                    ImGui::Separator();
                }
                else
                {
                    ImGui::Text(
                        "Player not found."
                    );
                }

                ImGui::End();
            }

            // ================= VISUAL POLISH PANEL =================
                    if (
                        appMode == AppMode::Editor &&
                        showVisualPolishPanel &&
                        !cinematicOverlay.enabled
                        )
                    {
                        ImGui::SetNextWindowPos(
                            ImVec2(
                                centerX,
                                topY + 360.0f
                            ),
                            ImGuiCond_FirstUseEver
                        );

                        ImGui::SetNextWindowSize(
                            ImVec2(
                                370.0f,
                                300.0f
                            ),
                            ImGuiCond_FirstUseEver
                        );

                        ImGui::Begin(
                            "Visual Polish"
                        );

                        ImGui::Text(
                            "Lighting and Presentation"
                        );

                        ImGui::Separator();

                        ImGui::Text(
                            "Runtime Game Mode"
                        );

                        int selectedMode =
                            static_cast<int>(
                                selectedRuntimeGameMode
                                );

                        if (
                            ImGui::Combo(
                                "Game Mode",
                                &selectedMode,
                                runtimeGameModeNames,
                                4
                            )
                            )
                        {
                            selectedRuntimeGameMode =
                                static_cast<RuntimeGameMode>(
                                    selectedMode
                                    );
                        }

                        ImGui::Text(
                            "Selected: %s",
                            GetSelectedRuntimeGameModeName()
                        );

                        ImGui::TextDisabled(
                            "Choose mode here, then press Play."
                        );

                        ImGui::Separator();

                        ImGui::Checkbox(
                            "Enable Day/Night Cycle",
                            &dayNightSystem.enabled
                        );

                        ImGui::SliderFloat(
                            "Time of Day",
                            &dayNightSystem.timeOfDay,
                            0.0f,
                            24.0f
                        );

                        ImGui::DragFloat(
                            "Cycle Speed",
                            &dayNightSystem.cycleSpeed,
                            0.01f,
                            0.01f,
                            5.0f
                        );

                        ImGui::Separator();

                        ImGui::Text(
                            "Weather / Atmosphere"
                        );

                        ImGui::Checkbox(
                            "Enable Weather System",
                            &weatherSystem.enabled
                        );

                        ImGui::Combo(
                            "Weather Preset",
                            &weatherSystem.presetIndex,
                            WeatherSystem::PresetNames,
                            WeatherSystem::PresetCount
                        );

                        ImGui::Checkbox(
                            "Show Weather Overlay",
                            &weatherSystem.showOverlay
                        );

                        ImGui::Text(
                            "Fog: %s",
                            weatherSystem.useFog ? "Enabled" : "Disabled"
                        );

                        ImGui::Text(
                            "Rain: %s",
                            weatherSystem.rainOverlay ? "Enabled" : "Disabled"
                        );

                        ImGui::Separator();

                        ImGui::Checkbox(
                            "Show Objective Markers",
                            &objectiveMarkerSystem.enabled
                        );

                        ImGui::Checkbox(
                            "Show Mini Map Radar",
                            &miniMapRadar.enabled
                        );

                        ImGui::DragFloat(
                            "Mini Map Range",
                            &miniMapRadar.range,
                            1.0f,
                            25.0f,
                            250.0f
                        );

                        ImGui::DragFloat(
                            "Mini Map Size",
                            &miniMapRadar.radius,
                            1.0f,
                            60.0f,
                            150.0f
                        );

                        ImGui::Checkbox(
                            "Show Gameplay Feedback FX",
                            &gameplayFeedbackFX.enabled
                        );

                        ImGui::Checkbox(
                            "Cinematic Mode",
                            &cinematicOverlay.enabled
                        );

                        ImGui::TextDisabled(
                            "Shortcut: F10"
                        );

                        ImGui::DragFloat(
                            "Black Bar Height",
                            &cinematicOverlay.blackBarHeight,
                            1.0f,
                            0.0f,
                            180.0f
                        );

                        ImGui::Separator();

                        if (
                            ImGui::Button(
                                "Golden Hour"
                            )
                            )
                        {
                            dayNightSystem.SetGoldenHour();
                        }

                        ImGui::SameLine();

                        if (
                            ImGui::Button(
                                "Night"
                            )
                            )
                        {
                            dayNightSystem.SetNight();
                        }

                        ImGui::SameLine();

                        if (
                            ImGui::Button(
                                "Day"
                            )
                            )
                        {
                            dayNightSystem.SetDay();
                        }

                        ImGui::End();
                    }
            // ================= SELECTED OBJECT PLACEMENT TOOLS =================
            if (showSelectedObjectToolsPanel)
            {
            ImGui::SetNextWindowPos(
                ImVec2(
                    1240.0f,
                    600.0f
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
            ImGui::SetNextWindowPos(
                ImVec2(rightX, topY + 710.0f),
                ImGuiCond_FirstUseEver
            );

            ImGui::SetNextWindowSize(
                ImVec2(rightWidth, 170.0f),
                ImGuiCond_FirstUseEver
            );
            ImGui::Begin("Selected Object Tools");

            if (selectedObject != nullptr)
            {
                ImGui::Text("Selected:");
                ImGui::Text("%s", selectedObject->name.c_str());

                ImGui::Separator();

                ImGui::Text(
                    "Position: %.2f, %.2f, %.2f",
                    selectedObject->transform.position.x,
                    selectedObject->transform.position.y,
                    selectedObject->transform.position.z
                );

                ImGui::Text(
                    "Scale: %.2f, %.2f, %.2f",
                    selectedObject->transform.scale.x,
                    selectedObject->transform.scale.y,
                    selectedObject->transform.scale.z
                );

                ImGui::Separator();

                if (ImGui::Button("Move Selected In Front Of Camera"))
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

                    glm::vec3 newPosition =
                        camera.Position +
                        forward * 6.0f;

                    newPosition.y =
                        selectedObject->transform.position.y;
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

                if (ImGui::Button("Snap Selected To Ground"))
                {
                    selectedObject->transform.position.y =
                        GetObjectTerrainY(
                            selectedObject->transform.position.x,
                            selectedObject->transform.position.z,
                            0.05f
                        );

                    std::cout
                        << "Selected object snapped to terrain."
                        << std::endl;
                }

                ImGui::Separator();

                if (ImGui::Button("Rotate Y +15"))
                {
                    selectedObject->transform.rotation.y +=
                        15.0f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Rotate Y -15"))
                {
                    selectedObject->transform.rotation.y -=
                        15.0f;
                }

                if (ImGui::Button("Rotate Y +90"))
                {
                    selectedObject->transform.rotation.y +=
                        90.0f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Random Y Rotation"))
                {
                    selectedObject->transform.rotation.y =
                        static_cast<float>(
                            rand() % 360
                            );
                }

                ImGui::Separator();

                if (ImGui::Button("Scale Up"))
                {
                    selectedObject->transform.scale *=
                        1.1f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Scale Down"))
                {
                    selectedObject->transform.scale *=
                        0.9f;
                }

                if (ImGui::Button("Random Small Scale"))
                {
                    float randomScale =
                        0.8f +
                        static_cast<float>(rand() % 41) / 100.0f;

                    selectedObject->transform.scale =
                        glm::vec3(
                            randomScale
                        );
                }

                ImGui::Separator();

                if (ImGui::Button("Mark As Collider"))
                {
                    selectedObject->isCollider =
                        true;

                    selectedObject->colliderRadius =
                        glm::max(
                            selectedObject->transform.scale.x,
                            selectedObject->transform.scale.z
                        ) * 0.8f;

                    std::cout
                        << "Selected object marked as collider."
                        << std::endl;
                }

                ImGui::SameLine();

                if (ImGui::Button("Remove Collider"))
                {
                    selectedObject->isCollider =
                        false;

                    std::cout
                        << "Selected object collider removed."
                        << std::endl;
                }

                ImGui::Text(
                    "Collider: %s",
                    selectedObject->isCollider ? "ON" : "OFF"
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
                    gameplayTypeIndex = 1;

                else if (selectedObject->editorGameplayType == "TriggerZone")
                    gameplayTypeIndex = 2;

                else if (selectedObject->editorGameplayType == "MonsterSpawn")
                    gameplayTypeIndex = 3;

                else if (selectedObject->editorGameplayType == "MusicGate")
                    gameplayTypeIndex = 4;

                else if (selectedObject->editorGameplayType == "MusicNPC")
                    gameplayTypeIndex = 5;

                else if (selectedObject->editorGameplayType == "Obstacle")
                    gameplayTypeIndex = 6;

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

                    std::cout
                        << "Gameplay type changed to: "
                        << selectedObject->editorGameplayType
                        << std::endl;
                }

                ImGui::Checkbox(
                    "Is Physical Collider",
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
                    "Gameplay Type is used by Play Mode systems."
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

                        std::cout
                            << "Applied gameplay role: "
                            << type
                            << std::endl;
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

                if (ImGui::Button("Make Trigger"))
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

                    std::cout
                        << "Gameplay role cleared."
                        << std::endl;
                }
            }
            else
            {
                ImGui::Text("No object selected.");
            }

            ImGui::End();
          }
          // ================= COLLISION DEBUG DRAW =================

          if (showCollisionDebug)
          {
              DrawCollisionDebugView(
                  scene,
                  selectedObject,
                  playerObject,
                  gizmoShader,
                  collisionCircleVAO,
                  collisionCircleVertexCount,
                  view,
                  projection,
                  collisionDebugYOffset,
                  collisionDebugLineWidth
              );
          }
          if (selectedObject != nullptr)
          {
              glLineWidth(4.0f);
              gizmoShader.use();

              glm::mat4 gizmoModel =
                  glm::translate(
                      glm::mat4(1.0f),
                      selectedObject->transform.position
                  );

              gizmoShader.setMat4("model", glm::value_ptr(gizmoModel));
              gizmoShader.setMat4("view", glm::value_ptr(view));
              gizmoShader.setMat4("projection", glm::value_ptr(projection));

              glBindVertexArray(gizmoVAO);

              gizmoShader.setVec3("axisColor", glm::vec3(1, 0, 0));
              glDrawArrays(GL_LINES, 0, 2);

              gizmoShader.setVec3("axisColor", glm::vec3(0, 1, 0));
              glDrawArrays(GL_LINES, 2, 2);

              gizmoShader.setVec3("axisColor", glm::vec3(0, 0, 1));
              glDrawArrays(GL_LINES, 4, 2);

              glBindVertexArray(0);
          }

          // ================= UNDO / REDO CAPTURE =================

          UpdateTransformHistoryCapture(
              window,
              scene,
              selectedObject,
              isDragging
          );
        }
        else
        {
            EditorUI::DrawCrosshair();
            ImGui::Begin("Play Mode Info");

            ImGui::Text("PLAY MODE ACTIVE");
            ImGui::Separator();

            ImGui::Text("Controls:");
            ImGui::Text("WASD - Move");
            ImGui::Text("SHIFT - Run");
            ImGui::Text("SPACE - Jump");
            ImGui::Text("Right Mouse - Look Around");
            ImGui::Text("TAB - Switch Editor / Play");
            ImGui::Separator();

            ImGui::Text(
                "Animation State: %s",
                thirdPersonController.GetAnimStateName()
            );

            ImGui::Text(
                "Grounded: %s",
                thirdPersonController.isGrounded ? "Yes" : "No"
            );

            ImGui::Text(
                "Speed: %.2f",
                thirdPersonController.currentMoveSpeed
            );
            if (showAnimatedRuntimeDebug)
            {
                ImGui::Separator();

                ImGui::Text(
                    "Animated Clip: %s",
                    animatedRuntimeClip.c_str()
                );

                ImGui::Text(
                    "Jump Lock: %.2f",
                    animatedJumpLockTimer
                );

                ImGui::Text(
                    "Animated Player: %s",
                    useAnimatedPlayerVisual ? "Enabled" : "Disabled"
                );
            }
            ImGui::Separator();
            ImGui::Text("Press Stop to return to editor.");
            ImGui::Separator();

            if (ImGui::Button("Save Editor Objects"))
            {
                SaveEditorObjects(
                    scene,
                    "SavedEditorObjects.txt"
                );
            }

            if (ImGui::Button("Load Editor Objects"))
            {
                LoadEditorObjects(
                    scene,
                    "SavedEditorObjects.txt",
                    &cube,
                    &shader,
                    &cubeMaterial,
                    selectedObject,
                    &woodenHouseModel,
                    &newHouseModel,
                    &pineTreeModel,
                    &commonTreeModel,
                    &rockModel,
                    &bushModel,
                    &woodLogModel,
                    &treeStumpModel,
                    &grassModel
                );
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
}
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    audioSystem.Stop(
        "walk_grass"
    );

    audioSystem.Stop(
        "run_grass"
    );
    audioSystem.Shutdown();
    glfwTerminate();

    return 0;

}


